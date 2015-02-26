#
# Copyright (C) 2011 EADS France, stephane duverger <stephane.duverger@eads.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
import utils

class Memory:
    def __init__(self, cpu, gdb):
        self.__gdb = gdb
        self.__cpu = cpu

    def __encode(self, val, n):
        fmt = "%."+str(n)+"x"
        out = fmt % (val)
        return out[-n:]

    def __encode_addr(self, addr):
        return self.__encode(addr, self.__cpu.sz)

    def __encode_val(self, val, n):
        return utils.revert_string_bytes(self.__encode(val, n*2))

    def __decode_val(self, val):
        return int(utils.revert_string_bytes(val), 16)

    def _flush(self):
        return

    def _resume(self):
        self._flush()

    def _quit(self):
        return

    def cr_sync(self):
        self.__cpu.sr["cr0"]._flush()
        self.__cpu.sr["cr3"]._flush()
        self.__cpu.sr["cr4"]._flush()

    def __read(self, addr, n):
        self.cr_sync()
        return self.__decode_val(self.__gdb.read_mem(self.__encode_addr(addr),n))

    def __write(self, addr, val, n):
        self.cr_sync()
        self.__gdb.write_mem(self.__encode_addr(addr),self.__encode_val(val,n),n)

    def read_byte(self, addr):
        return self.__read(addr, 1)
    def read_word(self, addr):
        return self.__read(addr, 2)
    def read_dword(self, addr):
        return self.__read(addr, 4)
    def read_qword(self, addr):
        return self.__read(addr, 8)

    def write_byte(self, addr, val):
        return self.__write(addr, val, 1)
    def write_word(self, addr, val):
        return self.__write(addr, val, 2)
    def write_dword(self, addr, val):
        return self.__write(addr, val, 4)
    def write_qword(self, addr, val):
        return self.__write(addr, val, 8)

    def pread(self, addr, sz):
        encoded = self.__encode(addr,16)+self.__encode(sz,16)
        return self.__gdb.read_pmem(encoded, sz)

    def pwrite(self, addr, data):
        sz = len(data)
        encoded = self.__encode(addr,16)+self.__encode(sz,16)
        self.__gdb.write_pmem(encoded, data, sz)

    def vread(self, addr, sz):
        self.cr_sync()
        encoded = self.__encode(addr,16)+self.__encode(sz,16)
        return self.__gdb.read_vmem(encoded, sz)

    def vwrite(self, addr, data):
        self.cr_sync()
        sz = len(data)
        encoded = self.__encode(addr,16)+self.__encode(sz,16)
        self.__gdb.write_vmem(encoded, data, sz)

    def translate(self, addr):
        self.cr_sync()
        paddr = self.__gdb.translate(self.__encode_addr(addr), self.__cpu.sz)
        return int(paddr, 16)

    def get_npte(self, addr):
        pte = self.__gdb.npg_get_pte(self.__encode_addr(addr), 8*2)
        return int(pte, 16)

    def set_npte(self, addr, value):
        encoded = self.__encode(addr,16)+self.__encode(value,16)
        self.__gdb.npg_set_pte(encoded)

    def nested_translate(self, addr):
        saddr = self.__gdb.npg_translate(self.__encode_addr(addr), 16)
        return int(saddr, 16)
