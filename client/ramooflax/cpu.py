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
import register
import gdb
import event
import breakpoints

from utils import Utils

class CPUFamily:
    AMD   = 0
    Intel = 1

class CPUException:
    divide_error       =  0
    debug              =  1
    nmi                =  2
    breakpoint         =  3
    overflow           =  4
    bound_range        =  5
    undef_opcode       =  6
    dev_not_avl        =  7
    double_fault       =  8
    copro_seg          =  9
    invalid_tss        = 10
    seg_not_present    = 11
    stack_fault        = 12
    general_protection = 13
    page_fault         = 14
    reserved           = 15
    fpu                = 16
    alignment_check    = 17
    machine_check      = 18
    simd               = 19

class CPU:
    def __init__(self, manufacturer, gdb):
        self.__gdb = gdb
        self.__filter = event.EventFilter()
        self.__last_reason = event.StopReason.gdb_int

        self.manufacturer = manufacturer
        self.sr = register.SR_x86(self.__gdb)
        self.lbr = register.LBR(self.__gdb)

        #not defined mode
        self.sz = None
        self.mode = None
        self.gpr = None
        self.breakpoints = breakpoints.BreakPoints(self, gdb, self.__filter)

    def __get_last_reason(self):
        return self.__last_reason

    def __set_exception_mask(self, mask):
        self.__gdb.write_exception_mask("%.8x" % (mask))

    def __get_exception_mask(self):
        return int(self.__gdb.read_exception_mask(), 16)

    def __set_cr_rd_mask(self, mask):
        self.__gdb.write_cr_read_mask("%.4x" % (mask))

    def __get_cr_rd__mask(self):
        return int(self.__gdb.read_cr_read_mask(), 16)

    def __set_cr_wr_mask(self, mask):
        self.__gdb.write_cr_write_mask("%.4x" % (mask))

    def __get_cr_wr_mask(self):
        return int(self.__gdb.read_cr_write_mask(), 16)

    def __segmem_location(self, seg, reg):
        if self.mode == 16:
            return (seg + (reg & 0xffff))
        elif self.mode == 32:
            return (seg + (reg & 0xffffffff))
        return reg

    def __update_mode(self, new_mode):
        self.mode = new_mode
        #XXX: gdb seems to need 4 bytes
        if self.mode == 32 or self.mode == 16:
            self.gpr = register.GPR_x86_32(self.__gdb)
            self.sz = 8
        elif self.mode == 64:
            self.gpr = register.GPR_x86_64(self.__gdb)
            self.sz = 16
        else:
            print "Unknown CPU mode: %d" % (self.mode)
            raise ValueError

    def __recv_stop(self):
        if Utils.debug:
            print "waiting stop reason"

        stop = self.__gdb.recv_stop()
        self.__last_reason = stop.reason

        if stop.mode != self.mode:
            self.__update_mode(stop.mode)

        # [ regN:regV, ... ]
        for g in stop.msg:
            n,v = g.split(':')
            self.gpr[int(n)]._update(v)

    def _recv_stop(self):
        self.__recv_stop()

    def _process_stop(self, vm):
        return self.__filter(self.__last_reason, vm)

    def _stop(self):
        self.__gdb.intr()

    def _resume(self, addr=None):
        self._flush()
        self.__gdb.resume(addr)

    def _singlestep(self, addr=None):
        self._flush()
        self.__gdb.singlestep(addr)

    def _quit(self):
        return

    def _flush(self):
        self.lbr._dirty()
        self.gpr._flush()
        self.sr._flush()

    def set_active_cr3(self, cr3, remember=False, affinity=None):
        self.__gdb.set_active_cr3("%.16x" % (cr3))
        if remember:
            self.__gdb.keep_active_cr3()
        if affinity != None:
            self.__gdb.set_affinity("%.2x" % (affinity))

    def del_active_cr3(self):
        self.__gdb.del_active_cr3()

    def filter_singlestep(self, hdl):
        self.breakpoints.filter(None, hdl)

    def release_singlestep(self, hdl):
        self.breakpoints.release()

    def filter_exception(self, n, hdl):
        if n > CPUException.simd:
            raise ValueError
        mask = self.__get_exception_mask() | (1<<n)
        self.__set_exception_mask(mask)
        self.__filter.register(event.StopReason.excp_de+n, hdl)

    def release_exception(self, n):
        if n > CPUException.simd:
            raise ValueError
        mask = self.__get_exception_mask() & ~(1<<n)
        self.__set_exception_mask(mask)
        self.__filter.unregister(event.StopReason.excp_de+n)

    def filter_read_cr(self, n, hdl):
        if n > 8:
            raise ValueError
        mask = self.__get_cr_rd_mask() | (1<<n)
        self.__set_cr_rd_mask(mask)
        self.__filter.register(event.StopReason.rd_cr0+n, hdl)

    def release_read_cr(self, n):
        if n > 8:
            raise ValueError
        mask = self.__get_cr_rd_mask() & ~(1<<n)
        self.__set_cr_rd_mask(mask)
        self.__filter.unregister(event.StopReason.rd_cr0+n)

    def filter_write_cr(self, n, hdl):
        if n > 8:
            raise ValueError
        mask = self.__get_cr_wr_mask() | (1<<n)
        self.__set_cr_wr_mask(mask)
        self.__filter.register(event.StopReason.wr_cr0+n, hdl)

    def release_write_cr(self, n):
        if n > 8:
            raise ValueError
        mask = self.__get_cr_wr_mask() & ~(1<<n)
        self.__set_cr_wr_mask(mask)
        self.__filter.unregister(event.StopReason.wr_cr0+n)

    def code_location(self):
        return self.__segmem_location(self.sr.cs, self.gpr.pc)

    def stack_location(self):
        return self.__segmem_location(self.sr.ss, self.gpr.stack)

    # xxx: modulo segment limit
    def linear(self, segment, offset):
        return self.__segmem_location(segment, offset)

    last_reason = property(__get_last_reason)
