/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <vmx_show.h>
#include <print.h>
#include <info_data.h>

extern info_data_t *info;

void vmx_show_basic_info()
{
   printf("\n- vmx basic info 0x%X\n"
	  "vmcs revision identifier   : %d\n"
	  "vmxon, vmcs size           : %d\n"
	  "physical width 32 limit    : %d\n"
	  "dual smm                   : %d\n"
	  "vmcs mem type (UC:0, WB:6) : %d\n"
	  "ins/outs insn info         : %d\n"
	  "true f1 settings           : %d\n"
	  ,info->vm.vmx_info.raw
	  ,info->vm.vmx_info.revision_id
	  ,info->vm.vmx_info.alloc
	  ,info->vm.vmx_info.pwidth32
	  ,info->vm.vmx_info.smm
	  ,info->vm.vmx_info.mem_type
	  ,info->vm.vmx_info.io_insn
	  ,info->vm.vmx_info.true_f1);
}

void vmx_show_misc_data()
{
   printf("\n- vmx misc data 0x%X\n"
	  "preempt rate        : %d\n"
	  "efer.lma in ia32e   : %d\n"
	  "halt activity       : %d\n"
	  "shut activity       : %d\n"
	  "sipi activity       : %d\n"
	  "cr3 target          : %d\n"
	  "msr max nr in list  : %d\n"
	  "smm mon             : %d\n"
	  "mseg rev            : %d\n"
	  ,info->vm.vmx_misc.raw
	  ,info->vm.vmx_misc.preempt_rate
	  ,info->vm.vmx_misc.lma
	  ,info->vm.vmx_misc.hlt
	  ,info->vm.vmx_misc.sht
	  ,info->vm.vmx_misc.ipi
	  ,info->vm.vmx_misc.cr3
	  ,(info->vm.vmx_misc.msr+1)*512
	  ,info->vm.vmx_misc.smm
	  ,info->vm.vmx_misc.mseg);
}

void vmx_show_pin_ctls()
{
   printf("\n- vmx pin based ctrls 0x%x\n"
	  "%d - vmexit on ext-int\n"
	  "%d - vmexit on nmi\n"
	  "%d - vnmi ctl\n"
	  "%d - enable preempt timer\n"
	  ,vm_exec_ctrls.pin.raw
	  ,vm_exec_ctrls.pin.eint
	  ,vm_exec_ctrls.pin.nmi
	  ,vm_exec_ctrls.pin.vnmi
	  ,vm_exec_ctrls.pin.preempt);
}

void vmx_show_proc_ctls()
{
   printf("\n- vmx proc1 based features 0x%x\n"
	  "%d - interrupt window exiting\n"
	  "%d - rdtsc offset\n"
	  "%d - hlt\n"
	  "%d - invlpg\n"
	  "%d - mwait\n"
	  "%d - rdpmc\n"
	  "%d - rdtsc\n"
	  "%d - wr cr3\n"
	  "%d - rd cr3\n"
	  "%d - wr cr8\n"
	  "%d - rd cr8\n"
	  "%d - TRP shadow\n"
	  "%d - nmi window\n"
	  "%d - mov dr\n"
	  "%d - unconditional IO\n"
	  "%d - use IO bitmaps\n"
	  "%d - monitor trap flag\n"
	  "%d - use MSR bitmaps\n"
	  "%d - monitor\n"
	  "%d - pause\n"
	  "%d - proc2\n"
	  ,vm_exec_ctrls.proc.raw
	  ,vm_exec_ctrls.proc.iwe
	  ,vm_exec_ctrls.proc.tsc
	  ,vm_exec_ctrls.proc.hlt
	  ,vm_exec_ctrls.proc.invl
	  ,vm_exec_ctrls.proc.mwait
	  ,vm_exec_ctrls.proc.rdpmc
	  ,vm_exec_ctrls.proc.rdtsc
	  ,vm_exec_ctrls.proc.cr3l
	  ,vm_exec_ctrls.proc.cr3s
	  ,vm_exec_ctrls.proc.cr8l
	  ,vm_exec_ctrls.proc.cr8s
	  ,vm_exec_ctrls.proc.tprs
	  ,vm_exec_ctrls.proc.nwe
	  ,vm_exec_ctrls.proc.mdr
	  ,vm_exec_ctrls.proc.ucio
	  ,vm_exec_ctrls.proc.usio
	  ,vm_exec_ctrls.proc.mtf
	  ,vm_exec_ctrls.proc.umsr
	  ,vm_exec_ctrls.proc.mon
	  ,vm_exec_ctrls.proc.pause
	  ,vm_exec_ctrls.proc.proc2);
}

void vmx_show_proc2_ctls()
{
   printf("\n- vmx proc2 based features 0x%x\n"
	  "%d - virtualize apic accesses\n"
	  "%d - enable EPT\n"
	  "%d - descriptor table exiting\n"
	  "%d - rdtscp raises #UD\n"
	  "%d - virtualize x2apic mode\n"
	  "%d - enable vpid\n"
	  "%d - wbinvd\n"
	  "%d - unrestricted guest\n"
	  "%d - apic register virtualization\n"
	  "%d - virtual interrupt delivery\n"
	  "%d - pause loop exiting\n"
	  "%d - rdrand\n"
	  "%d - invpcid raises #UD\n"
	  "%d - enable vm functions\n"
	  "%d - vmread/write to shadow vmcs\n"
	  "%d - EPT violation raises #VE\n"
	  ,vm_exec_ctrls.proc2.raw
	  ,vm_exec_ctrls.proc2.vapic
	  ,vm_exec_ctrls.proc2.ept
	  ,vm_exec_ctrls.proc2.dt
	  ,vm_exec_ctrls.proc2.rdtscp
	  ,vm_exec_ctrls.proc2.x2apic
	  ,vm_exec_ctrls.proc2.vpid
	  ,vm_exec_ctrls.proc2.wbinvd
	  ,vm_exec_ctrls.proc2.uguest
	  ,vm_exec_ctrls.proc2.vapic_reg
	  ,vm_exec_ctrls.proc2.vintr
	  ,vm_exec_ctrls.proc2.pause_loop
	  ,vm_exec_ctrls.proc2.rdrand
	  ,vm_exec_ctrls.proc2.invpcid
	  ,vm_exec_ctrls.proc2.vmfunc
	  ,vm_exec_ctrls.proc2.vmcs_shdw
	  ,vm_exec_ctrls.proc2.ept_ve);
}

void vmx_show_entry_ctls()
{
   printf("\n- vmx entry ctrls 0x%x\n"
	  "%d - load debugctl\n"
	  "%d - ia32e mode\n"
	  "%d - smm mode\n"
	  "%d - smm smi treatment\n"
	  "%d - load ia32 perf\n"
	  "%d - load ia32 pat\n"
	  "%d - load ia32 efer\n"
	  ,vm_entry_ctrls.entry.raw
	  ,vm_entry_ctrls.entry.load_dbgctl
	  ,vm_entry_ctrls.entry.ia32e
	  ,vm_entry_ctrls.entry.smm
	  ,vm_entry_ctrls.entry.dual
	  ,vm_entry_ctrls.entry.load_ia32_perf
	  ,vm_entry_ctrls.entry.load_ia32_pat
	  ,vm_entry_ctrls.entry.load_ia32_efer);
}

void vmx_show_exit_ctls()
{
   printf("\n- vmx exit ctrls 0x%x\n"
	  "%d - save debugctl\n"
	  "%d - host lmode\n"
	  "%d - load ia32e perf\n"
	  "%d - ack interrupt\n"
	  "%d - save ia32 pat\n"
	  "%d - load ia32 pat\n"
	  "%d - save ia32 efer\n"
	  "%d - load ia32 efer\n"
	  "%d - save preempt timer\n"
	  ,vm_exit_ctrls.exit.raw
	  ,vm_exit_ctrls.exit.save_dbgctl
	  ,vm_exit_ctrls.exit.host_lmode
	  ,vm_exit_ctrls.exit.load_ia32_perf
	  ,vm_exit_ctrls.exit.ack_int
	  ,vm_exit_ctrls.exit.save_ia32_pat
	  ,vm_exit_ctrls.exit.load_ia32_pat
	  ,vm_exit_ctrls.exit.save_ia32_efer
	  ,vm_exit_ctrls.exit.load_ia32_efer
	  ,vm_exit_ctrls.exit.save_preempt_timer);
}

void vmx_show_ept_cap()
{
   printf("\n- vmx extended page tables features 0x%X\n"
	  "%d - allow execute only ept entry\n"
	  "%d - page walk length of 4\n"
	  "%d - allow UC for ept structs\n"
	  "%d - allow WB for ept structs\n"
	  "%d - allow ept pde to map 2MB pages\n"
	  "%d - allow ept pdpte to map 1GB pages\n"
	  "%d - invept insn supported\n"
	  "%d - access & dirty flag in ept entry\n"
	  "%d - single context invept\n"
	  "%d - all context invept\n"
	  "%d - invvpid insn supported\n"
	  "%d - individual invvpid\n"
	  "%d - single context invvpid\n"
	  "%d - all context invvpid\n"
	  "%d - single context retaining globals invvpid\n"
	  ,info->vm.vmx_ept_cap.raw
	  ,info->vm.vmx_ept_cap.xo
	  ,info->vm.vmx_ept_cap.pwl4
	  ,info->vm.vmx_ept_cap.uc
	  ,info->vm.vmx_ept_cap.wb
	  ,info->vm.vmx_ept_cap.pg_2m
	  ,info->vm.vmx_ept_cap.pg_1g
	  ,info->vm.vmx_ept_cap.invept
	  ,info->vm.vmx_ept_cap.dirty
	  ,info->vm.vmx_ept_cap.invept_s
	  ,info->vm.vmx_ept_cap.invept_a
	  ,info->vm.vmx_ept_cap.invvpid
	  ,info->vm.vmx_ept_cap.invvpid_i
	  ,info->vm.vmx_ept_cap.invvpid_s
	  ,info->vm.vmx_ept_cap.invvpid_a
	  ,info->vm.vmx_ept_cap.invvpid_r);
}

/*
** Fixed fields
*/
void vmx_show_fixed_pin_ctls()
{
   printf("\n- vmx fixed pin based ctrls (fixed_1 0x%x, allow_1 0x%x)\n"
	  "(%d,%d) - vmexit on ext-int\n"
	  "(%d,%d) - vmexit on nmi\n"
	  "(%d,%d) - vnmi ctl\n"
	  "(%d,%d) - enable preempt timer\n"
	  ,info->vm.vmx_fx_pin.eax,info->vm.vmx_fx_pin.edx
	  ,info->vm.vmx_fx_pin.fixed_1.eint
	  ,info->vm.vmx_fx_pin.allow_1.eint
	  ,info->vm.vmx_fx_pin.fixed_1.nmi
	  ,info->vm.vmx_fx_pin.allow_1.nmi
	  ,info->vm.vmx_fx_pin.fixed_1.vnmi
	  ,info->vm.vmx_fx_pin.allow_1.vnmi
	  ,info->vm.vmx_fx_pin.fixed_1.preempt
	  ,info->vm.vmx_fx_pin.allow_1.preempt);
}

void vmx_show_fixed_proc_ctls()
{
   printf("\n- vmx fixed proc1 based features (fixed_1 0x%x, allow_1 0x%x)\n"
	  "(%d,%d) - interrupt window exiting\n"
	  "(%d,%d) - rdtsc offset\n"
	  "(%d,%d) - hlt\n"
	  "(%d,%d) - invlpg\n"
	  "(%d,%d) - mwait\n"
	  "(%d,%d) - rdpmc\n"
	  "(%d,%d) - rdtsc\n"
	  "(%d,%d) - wr cr3\n"
	  "(%d,%d) - rd cr3\n"
	  "(%d,%d) - wr cr8\n"
	  "(%d,%d) - rd cr8\n"
	  "(%d,%d) - TRP shadow\n"
	  "(%d,%d) - nmi window\n"
	  "(%d,%d) - mov dr\n"
	  "(%d,%d) - unconditional IO\n"
	  "(%d,%d) - use IO bitmaps\n"
	  "(%d,%d) - monitor trap flag\n"
	  "(%d,%d) - use MSR bitmaps\n"
	  "(%d,%d) - monitor\n"
	  "(%d,%d) - pause\n"
	  "(%d,%d) - proc2\n"
	  ,info->vm.vmx_fx_proc.eax, info->vm.vmx_fx_proc.edx
	  ,info->vm.vmx_fx_proc.fixed_1.iwe
	  ,info->vm.vmx_fx_proc.allow_1.iwe
	  ,info->vm.vmx_fx_proc.fixed_1.tsc
	  ,info->vm.vmx_fx_proc.allow_1.tsc
	  ,info->vm.vmx_fx_proc.fixed_1.hlt
	  ,info->vm.vmx_fx_proc.allow_1.hlt
	  ,info->vm.vmx_fx_proc.fixed_1.invl
	  ,info->vm.vmx_fx_proc.allow_1.invl
	  ,info->vm.vmx_fx_proc.fixed_1.mwait
	  ,info->vm.vmx_fx_proc.allow_1.mwait
	  ,info->vm.vmx_fx_proc.fixed_1.rdpmc
	  ,info->vm.vmx_fx_proc.allow_1.rdpmc
	  ,info->vm.vmx_fx_proc.fixed_1.rdtsc
	  ,info->vm.vmx_fx_proc.allow_1.rdtsc
	  ,info->vm.vmx_fx_proc.fixed_1.cr3l
	  ,info->vm.vmx_fx_proc.allow_1.cr3l
	  ,info->vm.vmx_fx_proc.fixed_1.cr3s
	  ,info->vm.vmx_fx_proc.allow_1.cr3s
	  ,info->vm.vmx_fx_proc.fixed_1.cr8l
	  ,info->vm.vmx_fx_proc.allow_1.cr8l
	  ,info->vm.vmx_fx_proc.fixed_1.cr8s
	  ,info->vm.vmx_fx_proc.allow_1.cr8s
	  ,info->vm.vmx_fx_proc.fixed_1.tprs
	  ,info->vm.vmx_fx_proc.allow_1.tprs
	  ,info->vm.vmx_fx_proc.fixed_1.nwe
	  ,info->vm.vmx_fx_proc.allow_1.nwe
	  ,info->vm.vmx_fx_proc.fixed_1.mdr
	  ,info->vm.vmx_fx_proc.allow_1.mdr
	  ,info->vm.vmx_fx_proc.fixed_1.ucio
	  ,info->vm.vmx_fx_proc.allow_1.ucio
	  ,info->vm.vmx_fx_proc.fixed_1.usio
	  ,info->vm.vmx_fx_proc.allow_1.usio
	  ,info->vm.vmx_fx_proc.fixed_1.mtf
	  ,info->vm.vmx_fx_proc.allow_1.mtf
	  ,info->vm.vmx_fx_proc.fixed_1.umsr
	  ,info->vm.vmx_fx_proc.allow_1.umsr
	  ,info->vm.vmx_fx_proc.fixed_1.mon
	  ,info->vm.vmx_fx_proc.allow_1.mon
	  ,info->vm.vmx_fx_proc.fixed_1.pause
	  ,info->vm.vmx_fx_proc.allow_1.pause
	  ,info->vm.vmx_fx_proc.fixed_1.proc2
	  ,info->vm.vmx_fx_proc.allow_1.proc2);
}

void vmx_show_fixed_proc2_ctls()
{
   printf("\n- vmx fixed proc2 based features (fixed_1 0x%x, allow_1 0x%x)\n"
	  "(%d,%d) - virtualize apic accesses\n"
	  "(%d,%d) - enable EPT\n"
	  "(%d,%d) - descriptor table exiting\n"
	  "(%d,%d) - rdtscp raises #UD\n"
	  "(%d,%d) - virtualize x2apic mode\n"
	  "(%d,%d) - enable vpid\n"
	  "(%d,%d) - wbinvd\n"
	  "(%d,%d) - unrestricted guest\n"
	  "(%d,%d) - apic register virtualization\n"
	  "(%d,%d) - virtual interrupt delivery\n"
	  "(%d,%d) - pause loop exiting\n"
	  "(%d,%d) - rdrand\n"
	  "(%d,%d) - invpcid raises #UD\n"
	  "(%d,%d) - enable vm functions\n"
	  "(%d,%d) - vmread/write to shadow vmcs\n"
	  "(%d,%d) - EPT violation raises #VE\n"
	  ,info->vm.vmx_fx_proc2.eax,info->vm.vmx_fx_proc2.edx
	  ,info->vm.vmx_fx_proc2.fixed_1.vapic
	  ,info->vm.vmx_fx_proc2.allow_1.vapic
	  ,info->vm.vmx_fx_proc2.fixed_1.ept
	  ,info->vm.vmx_fx_proc2.allow_1.ept
	  ,info->vm.vmx_fx_proc2.fixed_1.dt
	  ,info->vm.vmx_fx_proc2.allow_1.dt
	  ,info->vm.vmx_fx_proc2.fixed_1.rdtscp
	  ,info->vm.vmx_fx_proc2.allow_1.rdtscp
	  ,info->vm.vmx_fx_proc2.fixed_1.x2apic
	  ,info->vm.vmx_fx_proc2.allow_1.x2apic
	  ,info->vm.vmx_fx_proc2.fixed_1.vpid
	  ,info->vm.vmx_fx_proc2.allow_1.vpid
	  ,info->vm.vmx_fx_proc2.fixed_1.wbinvd
	  ,info->vm.vmx_fx_proc2.allow_1.wbinvd
	  ,info->vm.vmx_fx_proc2.fixed_1.uguest
	  ,info->vm.vmx_fx_proc2.allow_1.uguest
	  ,info->vm.vmx_fx_proc2.fixed_1.vapic_reg
	  ,info->vm.vmx_fx_proc2.allow_1.vapic_reg
	  ,info->vm.vmx_fx_proc2.fixed_1.vintr
	  ,info->vm.vmx_fx_proc2.allow_1.vintr
	  ,info->vm.vmx_fx_proc2.fixed_1.pause_loop
	  ,info->vm.vmx_fx_proc2.allow_1.pause_loop
	  ,info->vm.vmx_fx_proc2.fixed_1.rdrand
	  ,info->vm.vmx_fx_proc2.allow_1.rdrand
	  ,info->vm.vmx_fx_proc2.fixed_1.invpcid
	  ,info->vm.vmx_fx_proc2.allow_1.invpcid
	  ,info->vm.vmx_fx_proc2.fixed_1.vmfunc
	  ,info->vm.vmx_fx_proc2.allow_1.vmfunc
	  ,info->vm.vmx_fx_proc2.fixed_1.vmcs_shdw
	  ,info->vm.vmx_fx_proc2.allow_1.vmcs_shdw
	  ,info->vm.vmx_fx_proc2.fixed_1.ept_ve
	  ,info->vm.vmx_fx_proc2.allow_1.ept_ve);
}

void vmx_show_fixed_entry_ctls()
{
   printf("\n- vmx fixed entry ctrls (fixed_1 0x%x, allow_1 0x%x)\n"
	  "(%d,%d) - load debugctl\n"
	  "(%d,%d) - ia32e mode\n"
	  "(%d,%d) - smm mode\n"
	  "(%d,%d) - smm smi treatment\n"
	  "(%d,%d) - load ia32 perf\n"
	  "(%d,%d) - load ia32 pat\n"
	  "(%d,%d) - load ia32 efer\n"
	  ,info->vm.vmx_fx_entry.eax,info->vm.vmx_fx_entry.edx
	  ,info->vm.vmx_fx_entry.fixed_1.load_dbgctl
	  ,info->vm.vmx_fx_entry.allow_1.load_dbgctl
	  ,info->vm.vmx_fx_entry.fixed_1.ia32e
	  ,info->vm.vmx_fx_entry.allow_1.ia32e
	  ,info->vm.vmx_fx_entry.fixed_1.smm
	  ,info->vm.vmx_fx_entry.allow_1.smm
	  ,info->vm.vmx_fx_entry.fixed_1.dual
	  ,info->vm.vmx_fx_entry.allow_1.dual
	  ,info->vm.vmx_fx_entry.fixed_1.load_ia32_perf
	  ,info->vm.vmx_fx_entry.allow_1.load_ia32_perf
	  ,info->vm.vmx_fx_entry.fixed_1.load_ia32_pat
	  ,info->vm.vmx_fx_entry.allow_1.load_ia32_pat
	  ,info->vm.vmx_fx_entry.fixed_1.load_ia32_efer
	  ,info->vm.vmx_fx_entry.allow_1.load_ia32_efer);
}

void vmx_show_fixed_exit_ctls()
{
   printf("\n- vmx fixed exit ctrls (fixed_1 0x%x, allow_1 0x%x)\n"
	  "(%d,%d) - save debugctl\n"
	  "(%d,%d) - host lmode\n"
	  "(%d,%d) - load ia32e perf\n"
	  "(%d,%d) - ack interrupt\n"
	  "(%d,%d) - save ia32 pat\n"
	  "(%d,%d) - load ia32 pat\n"
	  "(%d,%d) - save ia32 efer\n"
	  "(%d,%d) - load ia32 efer\n"
	  "(%d,%d) - save preempt timer\n"
	  ,info->vm.vmx_fx_exit.eax, info->vm.vmx_fx_exit.edx
	  ,info->vm.vmx_fx_exit.fixed_1.save_dbgctl
	  ,info->vm.vmx_fx_exit.allow_1.save_dbgctl
	  ,info->vm.vmx_fx_exit.fixed_1.host_lmode
	  ,info->vm.vmx_fx_exit.allow_1.host_lmode
	  ,info->vm.vmx_fx_exit.fixed_1.load_ia32_perf
	  ,info->vm.vmx_fx_exit.allow_1.load_ia32_perf
	  ,info->vm.vmx_fx_exit.fixed_1.ack_int
	  ,info->vm.vmx_fx_exit.allow_1.ack_int
	  ,info->vm.vmx_fx_exit.fixed_1.save_ia32_pat
	  ,info->vm.vmx_fx_exit.allow_1.save_ia32_pat
	  ,info->vm.vmx_fx_exit.fixed_1.load_ia32_pat
	  ,info->vm.vmx_fx_exit.allow_1.load_ia32_pat
	  ,info->vm.vmx_fx_exit.fixed_1.save_ia32_efer
	  ,info->vm.vmx_fx_exit.allow_1.save_ia32_efer
	  ,info->vm.vmx_fx_exit.fixed_1.load_ia32_efer
	  ,info->vm.vmx_fx_exit.allow_1.load_ia32_efer
	  ,info->vm.vmx_fx_exit.fixed_1.save_preempt_timer
	  ,info->vm.vmx_fx_exit.allow_1.save_preempt_timer);
}
