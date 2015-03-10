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
import log
import inspect, struct

def in_completer():
    return (inspect.stack()[3][3] == "attr_matches")

def revert_string_bytes(s):
    rs = []
    for i in range(len(s)/2):
        rs.append(s[-2:])
        s = s[:-2]
    return "".join(rs)

def stackdump(vm, n):
    sz   = vm.cpu.mode/8
    data = vm.mem.vread(vm.cpu.stack_location(), n*sz)
    fmt  = "%#0*x"
    if vm.cpu.mode == 16:
        sf = "H"
    elif vm.cpu.mode == 32:
        sf ="L"
    else:
        sf = "Q"
    msg = ""
    for x in struct.unpack("<"+sf*n, data):
        msg += fmt % ((sz*2)+2, x)
    return msg

def backtrace(vm, n=1):
    sz = vm.cpu.mode/8
    ft = "<"+{16:"H",32:"L",64:"Q"}[vm.cpu.mode]
    ip = []
    if n == 0:
        sp = vm.cpu.linear(vm.cpu.sr.ss, vm.cpu.gpr.stack)
        return struct.unpack(ft, vm.mem.vread(sp,sz))[0]
    bp = vm.cpu.linear(vm.cpu.sr.ss, vm.cpu.gpr.frame)
    for x in xrange(n):
        ip.append(struct.unpack(ft, vm.mem.vread(bp+sz, sz))[0])
        bp = struct.unpack(ft, vm.mem.vread(bp, sz))[0]
    return ip

def disassemble(vm, disasm, start, sz=15):
    end = start + sz
    dump = vm.mem.vread(start, sz)
    off = 0
    msg = ""
    while start < end:
        insn = disasm(start, dump[off:off+15])
        if insn is None:
            break
        msg   += "%.8x\t%s\n" % (start, insn)
        off   += insn.length
        start += insn.length
    return msg
