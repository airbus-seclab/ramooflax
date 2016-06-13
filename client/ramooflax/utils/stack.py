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
import struct

def stackargs(vm, n):
    sz   = vm.cpu.mode.addr_sz/8
    data = vm.mem.vread(vm.cpu.stack_location(), n*sz)
    sf   = {16:"H",32:"L",64:"Q"}[vm.cpu.mode.addr_sz]
    return struct.unpack("<"+sf*n, data)

def stackdump(vm, n):
    sz  = vm.cpu.mode.addr_sz/8
    fmt = "%#0*x"
    msg = ""
    for x in stackargs(vm,n):
        msg += fmt % ((sz*2)+2, x)
    return msg

def backtrace(vm, n=1):
    sz = vm.cpu.mode.addr_sz/8
    ft = "<"+{16:"H",32:"L",64:"Q"}[vm.cpu.mode.addr_sz]
    ip = []
    if n == 0:
        sp = vm.cpu.linear(vm.cpu.sr.ss_base, vm.cpu.gpr.stack)
        return struct.unpack(ft, vm.mem.vread(sp,sz))[0]
    bp = vm.cpu.linear(vm.cpu.sr.ss_base, vm.cpu.gpr.frame)
    for x in xrange(n):
        ip.append(struct.unpack(ft, vm.mem.vread(bp+sz, sz))[0])
        bp = struct.unpack(ft, vm.mem.vread(bp, sz))[0]
    return ip
