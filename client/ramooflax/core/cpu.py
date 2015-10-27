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
import register
import gdb
import event
import breakpoints
import log

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

class CPUFilter:
    npf   = (1<<0)
    hyp   = (1<<1)
    cpuid = (1<<2)

class CPUMode:
    rmode   = 1
    v80086  = 2
    pmode16 = 3
    pmode32 = 4
    lmode16 = 5
    lmode32 = 6
    lmode64 = 7

    pg32 = 1
    pae  = 2
    pg64 = 3

    def __init__(self, mode):
        self._exe_str = {0:"(unknown)",
                         CPUMode.rmode:"real mode 16",
                         CPUMode.v80086:"virtual 8086 16",
                         CPUMode.pmode16:"protected (legacy) 16",
                         CPUMode.pmode32:"protected (legacy) 32",
                         CPUMode.lmode16:"long mode 16",
                         CPUMode.lmode32:"long mode 32",
                         CPUMode.lmode64:"long mode 64",
                         }

        self._pg_str = {0:"disabled",
                        CPUMode.pg32:"32",
                        CPUMode.pae:"PAE",
                        CPUMode.pg64:"64",
                        }

        self.exe     = mode & 0xff
        self.pg      = (mode >> 8) & 0xff
        self.addr_sz = (mode >> 16) & 0xff
        self.nibbles = self.addr_sz/4

    def __str__(self):
        x_s = self._exe_str.get(self.exe, "(unknown)")
        p_s = self._pg_str.get(self.pg, "(unknown)")
        return "%s bits / paging %s / addr %d" % (x_s,p_s, self.addr_sz)

class CPU:
    def __init__(self, manufacturer, gdb):
        self.__gdb = gdb
        self.__filter = event.EventFilter()
        self.__last_reason = event.StopReason()

        self.manufacturer = manufacturer
        self.sr = register.SR_x86(self.__gdb)
        self.lbr = register.LBR(self.__gdb)
        self.fault = register.Fault(self.__gdb)
        self.msr = register.MSR(self.__gdb)

        #not defined mode
        self.mode = None
        self.gpr = None
        self.breakpoints = breakpoints.BreakPoints(self, gdb, self.__filter)

    def __get_last_reason(self):
        return self.__last_reason

    # exceptions filtering
    def __set_exception_mask(self, mask):
        self.__gdb.write_exception_mask("%.8x" % (mask))

    def __get_exception_mask(self):
        return int(self.__gdb.read_exception_mask(), 16)

    # control registers filtering
    def __set_cr_rd_mask(self, mask):
        self.__gdb.write_cr_read_mask("%.4x" % (mask))

    def __get_cr_rd_mask(self):
        return int(self.__gdb.read_cr_read_mask(), 16)

    def __set_cr_wr_mask(self, mask):
        self.__gdb.write_cr_write_mask("%.4x" % (mask))

    def __get_cr_wr_mask(self):
        return int(self.__gdb.read_cr_write_mask(), 16)

    # cpu mode
    def __get_cpu_mode(self):
        return int(self.__gdb.get_cpu_mode(), 16)

    # various filters
    def __set_filter_mask(self, mask):
        self.__gdb.write_filter_mask("%.16x" % (mask))

    def __get_filter_mask(self):
        return int(self.__gdb.read_filter_mask(), 16)

    def __filter_mask_add(self, v):
        mask = self.__get_filter_mask() | v
        self.__set_filter_mask(mask)

    def __filter_mask_del(self, v):
        mask = self.__get_filter_mask() & ~(v)
        self.__set_filter_mask(mask)

    def __segmem_location(self, base, reg):
        if self.mode.addr_sz == 16:
            return (base + (reg & 0xffff))
        elif self.mode.addr_sz == 32:
            return (base + (reg & 0xffffffff))
        return reg

    def __update_mode(self):
        mode = CPUMode(self.__get_cpu_mode())

        if self.mode is None or self.mode.addr_sz != mode.addr_sz:
            if mode.addr_sz == 32 or mode.addr_sz == 16:
                self.gpr = register.GPR_x86_32(self.__gdb)
            elif mode.addr_sz == 64:
                self.gpr = register.GPR_x86_64(self.__gdb)
            else:
                log.log("error", "Unknown CPU mode: %s" % (mode))
                raise ValueError

        self.mode = mode

    def has_pending_reason(self):
        return not self.__gdb.stop_pool_empty()

    def __recv_stop(self):
        log.log("cpu", "waiting stop reason")

        stop = self.__gdb.recv_stop()
        self.__last_reason = stop
        self.__update_mode()

        # [ regN:regV, ... ]
        for g in stop.msg:
            n,v = g.split(':')
            self.gpr[int(n)]._update(v)

    def _recv_stop(self):
        self.__recv_stop()

    def _process_stop(self, vm):
        return self.__filter(self.__last_reason.reason, vm)

    def _stop(self):
        self.__gdb.intr()

    def _resume(self, addr=None, sstep=False):
        self._flush()
        if sstep:
            self.__gdb.singlestep(addr)
        else:
            self.__gdb.resume(addr)

    def _quit(self):
        self.__set_exception_mask(0)
        self.__set_filter_mask(0)
        self.__set_cr_rd_mask(0)
        self.__set_cr_wr_mask(0)
        return

    def _flush(self):
        self.fault._dirty()
        self.lbr._dirty()
        self.gpr._flush()
        self.sr._flush()

    def nested_fault_is_final(self):
        if self.manufacturer == CPUFamily.Intel:
            return (self.fault.npf_err & 1<<8)
        else:
            return (self.fault.npf_err & 1<<32)

    def rdtsc(self):
        return int(self.__gdb.rdtsc(), 16)

    def can_cli(self):
        return (int(self.__gdb.can_cli(), 16) == 1)

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

    def clear_idt_event(self):
        self.__gdb.clear_idt_event()

    def get_idt_event(self):
        return event.IdtEvent(int(self.__gdb.get_idt_event(), 16))

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

    def filter_npf(self, hdl):
        self.__filter_mask_add(CPUFilter.npf)
        self.__filter.register(event.StopReason.npf, hdl)

    def release_npf(self):
        self.__filter_mask_del(CPUFilter.npf)
        self.__filter.unregister(event.StopReason.npf)

    def filter_hypercall(self, hdl):
        self.__filter_mask_add(CPUFilter.hyp)
        self.__filter.register(event.StopReason.hyp, hdl)

    def release_hypercall(self):
        self.__filter_mask_del(CPUFilter.hyp)
        self.__filter.unregister(event.StopReason.hyp)

    def filter_cpuid(self, hdl):
        self.__filter_mask_add(CPUFilter.cpuid)
        self.__filter.register(event.StopReason.cpuid, hdl)

    def release_cpuid(self):
        self.__filter_mask_del(CPUFilter.cpuid)
        self.__filter.unregister(event.StopReason.cpuid)

    def code_location(self):
        return self.__segmem_location(self.sr.cs_base, self.gpr.pc)

    def stack_location(self):
        return self.__segmem_location(self.sr.ss_base, self.gpr.stack)

    # xxx: modulo segment limit
    def linear(self, base, offset):
        return self.__segmem_location(base, offset)

    last_reason = property(__get_last_reason)
