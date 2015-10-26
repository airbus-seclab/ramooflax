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
import struct

class SegmentDescriptorType:
    tss_avl_16   = 0x01
    ldt          = 0x02
    tss_busy_16  = 0x03
    call_gate_16 = 0x04
    task_gate    = 0x05
    intr_gate_16 = 0x06
    trap_gate_16 = 0x07
    tss_avl      = 0x09
    tss_busy     = 0x0b
    call_gate    = 0x0c
    intr_gate    = 0x0e
    trap_gate    = 0x0f                 
    data_r       = 0x10
    data_ra      = 0x11
    data_rw      = 0x12
    data_rwa     = 0x13
    data_er      = 0x14
    data_era     = 0x15
    data_erw     = 0x16
    data_erwa    = 0x17
    code_x       = 0x18
    code_xa      = 0x19
    code_xr      = 0x1a
    code_xra     = 0x1b
    code_cx      = 0x1c
    code_cxa     = 0x1d
    code_cxr     = 0x1e
    code_cxra    = 0x1f

    def __init__(self, value):
        self.value = value
        self._dico = {
            SegmentDescriptorType.tss_avl_16   :"tss_avl_16",
            SegmentDescriptorType.ldt          :"ldt",
            SegmentDescriptorType.tss_busy_16  :"tss_busy_16",
            SegmentDescriptorType.call_gate_16 :"call_gate_16",
            SegmentDescriptorType.task_gate    :"task_gate",
            SegmentDescriptorType.intr_gate_16 :"intr_gate_16",
            SegmentDescriptorType.trap_gate_16 :"trap_gate_16",
            SegmentDescriptorType.tss_avl      :"tss_avl",
            SegmentDescriptorType.tss_busy     :"tss_busy",
            SegmentDescriptorType.call_gate    :"call_gate",
            SegmentDescriptorType.intr_gate    :"intr_gate",
            SegmentDescriptorType.trap_gate    :"trap_gate",
            SegmentDescriptorType.data_r       :"data_r",
            SegmentDescriptorType.data_ra      :"data_ra",
            SegmentDescriptorType.data_rw      :"data_rw",
            SegmentDescriptorType.data_rwa     :"data_rwa",
            SegmentDescriptorType.data_er      :"data_er",
            SegmentDescriptorType.data_era     :"data_era",
            SegmentDescriptorType.data_erw     :"data_erw",
            SegmentDescriptorType.data_erwa    :"data_erwa",
            SegmentDescriptorType.code_x       :"code_x",
            SegmentDescriptorType.code_xa      :"code_xa",
            SegmentDescriptorType.code_xr      :"code_xr",
            SegmentDescriptorType.code_xra     :"code_xra",
            SegmentDescriptorType.code_cx      :"code_cx",
            SegmentDescriptorType.code_cxa     :"code_cxa",
            SegmentDescriptorType.code_cxr     :"code_cxr",
            SegmentDescriptorType.code_cxra    :"code_cxra",
            }

    def __eq__(self, t):
        return self.value == t

    def __str__(self):
        return self._dico.get(self.value, "unknown")

class Descriptor32:
    def __init__(self, raw):
        if type(raw) is str:
            if len(raw) != 8:
                raise ValueError
            self.raw = struct.unpack("<Q",raw)[0]
        elif type(raw) is int or type(raw) is long:
            self.raw = raw
        else:
            raise TypeError

class SegmentDescriptor(Descriptor32):
    def __init__(self, raw):
        Descriptor32.__init__(self, raw)

        self.type  = SegmentDescriptorType((self.raw>>40) & 0x1f)
        self.base  = ((self.raw>>32) & 0xff000000) | ((self.raw>>16) & 0xffffff)
        self.limit = ((self.raw>>32) & 0xf0000) | ((self.raw & 0xffff))
        self.dpl   = (self.raw>>45) & 0x3
        self.p = (self.raw>>47) & 0x1
        self.l = (self.raw>>53) & 0x1
        self.d = (self.raw>>54) & 0x1
        self.g = (self.raw>>55) & 0x1

        self.size = self.limit
        if self.g:
            self.size = (self.size<<12) + (1<<12) - 1

    def system(self):
        return (self.type.value < 0x10)

    def bar(self):
        return "-"*89+"\n"

    def hdr(self):
        fmt = "%-6s | %-6s | %-10s | %-7s | %-10s | %-13s | %-3s | %s | %s | %s | %s\n"
        hdr = fmt % ("Index","Offset","Base","Limit","Size","Type","DPL","P","L","D","G")
        return self.bar()+hdr+self.bar()

    def body(self):
        fmt = " | 0x%.8x | 0x%.5x | 0x%.8x | %-13s |  %d  | %d | %d | %d | %d\n"
        return fmt % (self.base,self.limit,self.size,self.type,
                      self.dpl,self.p,self.l,self.d,self.g)

    def __str__(self):
        return self.hdr()+"(none) | (none)"+self.body()+self.bar()


class InterruptDescriptor(Descriptor32):
    def __init__(self, raw):
        Descriptor32.__init__(self, raw)

        self.type     = SegmentDescriptorType((self.raw>>40) & 0xf)
        self.selector = (self.raw>>16) & 0xffff
        self.offset   = ((self.raw>>32) & 0xffff0000) | ((self.raw & 0xffff))
        self.dpl      = (self.raw>>45) & 0x3
        self.p        = (self.raw>>47) & 0x1

        if self.type == SegmentDescriptorType.task_gate:
            self.offset = 0

    def bar(self):
        return "-"*63+"\n"

    def hdr(self):
        fmt = "%-6s | %-6s | %-6s | %-10s | %-13s | %-3s | %s\n"
        hdr = fmt % ("Index","Offset","Select","Offset","Type","DPL","P")
        return self.bar()+hdr+self.bar()

    def body(self):
        fmt = " | 0x%.4x | 0x%.8x | %-13s |  %d  | %d\n"
        return fmt % (self.selector,self.offset,self.type,self.dpl,self.p)

    def __str__(self):
        return self.hdr()+"(none) | (none)"+self.body()+self.bar()
