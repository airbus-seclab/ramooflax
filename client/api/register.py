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
from utils import *

class Register(object):
    def __init__(self, idx, name, size, rw_ops):
        self.__idx = idx
        self.__name = name
        self.__size = size
        self.__nibbles = size*2
        self.__read, self.__write = rw_ops
        self.__value = 0
        self.__rd = False
        self.__wr = False

    def __get_nibbles(self):
        return self.__nibbles

    def __read_update(self, value):
        self.__value = value
        self.__rd = True
        self.__wr = False

    def __write_update(self, value):
        self.__value = value
        self.__rd = True
        self.__wr = True

    def __write_commit(self):
        self.__wr = False
        return self.__value

    def __need_commit(self):
        return self.__wr

    def __need_sync(self):
        return not self.__rd

    def __dirty(self):
        self.__rd = False

    def __get_fmt(self):
        return "%."+str(self.nibbles)+"x"

    def __get_fmt_full(self):
        return "0x"+self.__get_fmt()

    def __encode(self):
        s = self.__get_fmt() % (self.__value)
        return utils.revert_string_bytes(s)

    def __decode(self, s):
        return int(utils.revert_string_bytes(s), 16)

    def _update(self, gdbreg):
        self.__read_update(self.__decode(gdbreg))

    def _read(self):
        if self.__need_sync():
            self._update(self.__read(self.__idx, self.__nibbles))
        return self.__value

    def _write(self, value):
        self.__write_update(value)

    def _commit(self):
        return self.__encode(self.__write_commit())

    def __send(self):
        self.__write(self.__idx, self._commit())

    def _flush(self):
        if self.__need_commit():
            self.__send()
        self.__dirty()

    def __repr__(self):
        return ("%-7s = "+self.__get_fmt_full()) % (self.__name, self._read())

    value   = property(_read, _write)
    nibbles = property(__get_nibbles)

class RegisterSet(object):
    def __init__(self, rw_ops, lregs, loc_pfx, stk_pfx):
        self.__nibbles = 0
        self.__nr = 0
        self.__read, self.__write = rw_ops[1]
        self.__list = []
        self.__dico = {}
        for regs in lregs:
            sz = regs[0]
            nm = regs[1]
            for n in nm:
                r = Register(self.__nr, n, sz, rw_ops[0])

                fget = lambda me, who = self.__nr: me.__read_reg(who)
                fset = lambda me, v, who = self.__nr: me.__write_reg(who,v)

                if n.find(loc_pfx) != -1:
                    setattr(self.__class__, "pc", property(fget, fset))
                    self.__dico.update({"pc":self.__nr})
                elif n.find(stk_pfx) != -1:
                    setattr(self.__class__, "stack", property(fget, fset))
                    self.__dico.update({"stack":self.__nr})

                setattr(self.__class__, n, property(fget, fset))
                self.__dico.update({n:self.__nr})
                self.__list.append(r)

                self.__nibbles += sz*2
                self.__nr += 1

    def __read_reg(self, who):
        if utils.in_completer():
            return
        return self.__list[who].value

    def __write_reg(self, who, v):
        self.__list[who].value = v

    def __getitem__(self, who):
        if type(who) is not int:
            who = self.__dico[who]
        return self.__list[who]

    def _flush(self):
        for r in self.__list:
            r._flush()

    def _read(self):
        gdball = self.__read(self.__nibbles)
        for r in self.__list:
            gdbreg = gdball[:r.nibbles]
            gdball = gdball[r.nibbles:]
            r._update(gdbreg)

    def _write(self):
        data = ""
        for r in self.__list:
            data += r._commit()
        self.__write(data)

    def __repr__(self):
        data = ""
        for r in self.__list:
            data += repr(r)+"\n"
        return data

class RegisterSet_x86(RegisterSet):
    def __init__(self, rw_ops, lregs):
        RegisterSet.__init__(self, rw_ops, lregs, "ip", "sp")

### System Registers on x86 (default to 64 bits)
class SR_x86(RegisterSet_x86):
    def __init__(self, gdb):
        rw_ops = ((gdb.read_sr,gdb.write_sr), (gdb.read_all_sr,gdb.write_all_sr))
        sys = (8,("cr0","cr2","cr3","cr4",
                  "dr0","dr1","dr2","dr3",
                  "dr6","dr7","dbgctl","efer",
                  "cs", "ss", "ds", "es","fs","gs",
                  "gdtr","idtr","ldtr","tr"))
        RegisterSet_x86.__init__(self, rw_ops, (sys,))

### General Purpose Registers on x86
class GPR_x86(RegisterSet_x86):
    def __init__(self, gdb, regs):
        rw_ops = ((gdb.read_gpr,gdb.write_gpr), (gdb.read_all_gpr,gdb.write_all_gpr))
        RegisterSet_x86.__init__(self, rw_ops, regs)

### General Purpose Registers on x86 32 bits
class GPR_x86_32(GPR_x86):
    def __init__(self, gdb):
        gprs = (4,("eax","ecx","edx","ebx","esp","ebp","esi","edi",
                   "eip","eflags"))
        segs = (2,("cs","ss","ds","es","fs","gs"))
        GPR_x86.__init__(self, gdb, (gprs,segs))

### General Purpose Registers on x86 64 bits
class GPR_x86_64(GPR_x86):
    def __init__(self, gdb):
        gprs = (8,("rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi",
                   "r8","r9","r10","r11","r12","r13","r14","r15",
                   "rip","rflags"))
        segs = (2,("cs","ss","ds","es","fs","gs"))
        GPR_x86.__init__(self, gdb, (gprs,segs))

### Last Branch Record
class LBR:
    def __init__(self, gdb):
        self.__gdb = gdb
        self.__active = False
        self.__dirty = True
        self.__values = { "from":0, "to":0, "from_excp":0, "to_excp":0 }

    def __update(self):
        if self.__dirty:
            gdblbr = self.__gdb.get_lbr()
            self.__values["from"] = int(gdblbr[:16], 16)
            self.__values["to"] = int(gdblbr[16:32], 16)
            self.__values["from_excp"] = int(gdblbr[32:48], 16)
            self.__values["to_excp"] = int(gdblbr[48:], 16)
            self.__dirty = False

    def _dirty(self):
        self.__dirty = True

    def enable(self):
        if not self.__active:
            self.__gdb.set_lbr()
            self.__active = True

    def disable(self):
        if self.__active:
           self.__gdb.del_lbr()
           self.__active = False

    def __repr__(self):
        s = ""
        if self.__active:
            self.__update()
            for k,v in self.__values.items():
                s += "%-10s = 0x%.16x\n" % (k,v)
        return s

    def __read(self, k):
        try:
            if self.__active:
                self.__update()
                return self.__values[k]
            else:
                print "Enable LBR first"
                raise ValueError
        except:
            print "invalid LBR key"
            raise
