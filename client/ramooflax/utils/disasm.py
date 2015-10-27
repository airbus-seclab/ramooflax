#
# Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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

def disassemble(vm, wrapper, start, sz=15):
    end = start + sz
    dump = vm.mem.vread(start, sz)
    off = 0
    msg = ""
    while start < end:
        insn = wrapper(start, dump[off:off+15])
        if insn is None:
            break
        byte   = dump[off:off+insn.length].encode("hex")
        msg   += "%.8x\t%-16s\t%s\n" % (start, byte, insn)
        off   += insn.length
        start += insn.length
    return msg
