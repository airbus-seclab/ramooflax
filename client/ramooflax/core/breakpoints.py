#
# Copyright (C) 2016 Airbus Group, stephane duverger <stephane.duverger@airbus.com>
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
import event
import log

class BreakPointType:
    x  = 1 # execution
    w  = 2 # data writes only
    rw = 4 # data read/write

class BreakPoint:
    def __init__(self, name, filtr, addr, knd, sz):
        self.name = name
        self.count = 1
        self.filter = filtr
        self.addr = addr
        self.type = knd
        self.size = sz

    def __repr__(self):
        if self.type == None or self.type == BreakPointType.x:
            s = "eXecute"
        elif self.type == BreakPointType.w:
            s = "Write"
        elif self.type == BreakPointType.rw:
            s = "Read/Write"

        return "%s 0x%x %s sz %d cnt %d" % \
            (self.name, self.addr, s, self.size, self.count)

class BreakPoints:
    def __init__(self, cpu, gdb, filtr):
        self.__filter = filtr
        self.__gdb = gdb
        self.__cpu = cpu
        self.__list = {}
        self.__auto_name = 0
        self.__filter.register(event.StopReason.gdb_trap, self.__trap_filter)
        self.__filter_sstep = self.__trap_sstep

    def __encode(self, val):
        fmt = "%."+str(self.__cpu.mode.gdb_nibbles)+"x"
        out = fmt % (val)
        return out[-self.__cpu.mode.gdb_nibbles:]

    def __trap_sstep(self, vm):
        log.log("brk", "Single Step Trap @ %s" % hex(vm.cpu.code_location()))
        return True

    def __trap_break(self, vm):
        log.log("brk", "Breakpoint @ %s" % hex(vm.cpu.code_location()))
        return True

    def __trap_filter(self, vm):
        log.log("brk", "trap filter")
        if (vm.cpu.sr.dr6 & (1<<14)) != 0:
            return self.__filter_sstep(vm)

        tp = None
        for i in range(0,4):
            if (vm.cpu.sr.dr6 & (1<<i)) != 0:
                if (vm.cpu.sr.dr7 & (1<<(i*2+1))) != 0:
                    conf = (vm.cpu.sr.dr7>>(16+i*4)) & 0xf;
                    ad = eval("vm.cpu.sr.dr"+str(i))
                    tp = (conf & 3)+1
                    ln = ((conf>>2) & 3)+1
                    break

        if tp is None:
            ad = vm.cpu.code_location()
            ln = 1

        for b in self.__list.values():
            if b.type == tp and b.addr == ad and b.size == ln:
                if b.filter is not None:
                    return b.filter(vm)
                return self.__trap_break(vm)

        log.log("brk", "Hardware breakpoint not found !")
        return True

    def __remove(self, b):
        enc = self.__encode(b.addr)
        if b.type is None:
            b.count -= 1
            if b.count > 0:
                return False
            self.__gdb.del_mem_break(enc)
        else:
            self.__gdb.del_hrd_break(enc, str(b.type), str(b.size))

        return True

    def __clear(self):
        for b in self.__list.values():
            while not self.__remove(b):
                continue
        self.__list = {}
        self.__auto_name = 0

    def __add(self, addr, name=None, filtr=None, kind=None, size=1):
        if name in self.__list:
            self.__list[name].count += 1
            return True

        if name is None:
            self.__auto_name += 1
            name = str(self.__auto_name)
        elif type(name) is not str:
            name = str(name)

        ssize = str(size)
        if size not in [1,2,4]:
            log.log("error", "unsupported breakpoint size %d" % ssize)
            raise ValueError
        elif (size == 2 and addr & 1) or (size == 4 and addr & 3):
            log.log("error", "addr should be aligned on %d boundary" % ssize)
            raise ValueError

        if kind is None:
            self.__gdb.set_mem_break(self.__encode(addr))
        elif kind in [BreakPointType.x,BreakPointType.w,BreakPointType.rw]:
            self.__gdb.set_hrd_break(self.__encode(addr), str(kind), ssize)
        else:
            log.log("error", "unsupported breakpoint type %s" % str(kind))
            raise ValueError

        b = BreakPoint(name, filtr, addr, kind, size)
        self.__list.update({name:b})
        return True

    def add_insn(self, addr, filtr=None, name=None):
        return self.__add(addr, name, filtr)

    def add_hw_insn(self, addr, filtr=None, name=None):
        return self.__add(addr, name, filtr, kind=BreakPointType.x)

    def add_data_w(self, addr, sz, filtr=None, name=None):
        return self.__add(addr, name, filtr, kind=BreakPointType.w, size=sz)

    def add_data_rw(self, addr, sz, filtr=None, name=None):
        return self.__add(addr, name, filtr, kind=BreakPointType.rw, size=sz)

    def remove(self, name=None):
        if name is None:
            self.__clear()
            return

        if type(name) is not str:
            name = str(name)

        if self.__list.has_key(name):
            b = self.__list[name]
            if self.__remove(b):
                self.__list.pop(name)

    def filter(self, name, filtr):
        if name is None:
            self.__filter_sstep = filtr
        else:
            if type(name) is not str:
                name = str(name)
            if self.__list.has_key(name):
                self.__list[name].filter = filtr

    def release(self, name=None):
        if name is None:
            self.__filter_sstep = self.__trap_sstep
        else:
            if type(name) is not str:
                name = str(name)
            if self.__list.has_key(name):
                self.__list[name].filter = None

    def __repr__(self):
        data = ""
        for b in self.__list.values():
            data += repr(b)+"\n"
        return data

    def count(self):
        return len(self.__list)
