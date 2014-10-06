/*
** Copyright (C) 2011 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#include <vmx_vmcs.h>
#include <vmx_vmm.h>
#include <vmx_guest.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void vmx_vmcs_host_state_init()
{
   vm_host_state.cr0.raw = get_cr0();
   vm_host_state.cr3.raw = get_cr3();
   vm_host_state.cr4.raw = get_cr4();

   rd_msr_ia32_sysenter_cs(vm_host_state.ia32_sysenter_cs);
   rd_msr_ia32_sysenter_esp(vm_host_state.ia32_sysenter_esp);
   rd_msr_ia32_sysenter_eip(vm_host_state.ia32_sysenter_eip);

   rd_msr_ia32_perf_glb_ctl(vm_host_state.ia32_perf);
   rd_msr_pat(vm_host_state.ia32_pat);
   rd_msr_ia32_efer(vm_host_state.ia32_efer);

   vm_host_state.rsp.raw = info->vmm.stack_bottom;
   vm_host_state.rip.raw = info->vmm.entry;

   vm_host_state.cs.raw = vmm_code_sel;

   vm_host_state.ss.raw = vmm_data_sel;
   vm_host_state.ds.raw = vmm_data_sel;
   vm_host_state.es.raw = vmm_data_sel;
   vm_host_state.fs.raw = vmm_data_sel;
   vm_host_state.gs.raw = vmm_data_sel;

   vm_host_state.tr.raw = vmm_tss_sel;

   vm_host_state.gdtr_base.raw = (offset_t)info->vmm.cpu.sg->gdt;
   vm_host_state.idtr_base.raw = (offset_t)info->vmm.cpu.sg->idt;
}

static void vmx_vmcs_entry_controls_init()
{
   vm_entry_ctrls.entry.load_ia32_perf = 1;
   vm_entry_ctrls.entry.load_ia32_pat  = 1;
   vm_entry_ctrls.entry.load_ia32_efer = 1;
   vm_entry_ctrls.entry.load_dbgctl    = 1;

   vm_entry_ctrls.msr_load_addr.raw = (offset_t)&info->vm.cpu.vmc->msr_entry_load;
}

static void vmx_vmcs_exit_controls_init()
{
   //vm_exit_ctrls.exit.save_preempt_timer = 1;

   vm_exit_ctrls.exit.host_lmode     = 1;
   vm_exit_ctrls.exit.load_ia32_perf = 1;
   vm_exit_ctrls.exit.load_ia32_pat  = 1;
   vm_exit_ctrls.exit.load_ia32_efer = 1;

   vm_exit_ctrls.exit.save_ia32_pat  = 1;
   vm_exit_ctrls.exit.save_ia32_efer = 1;
   vm_exit_ctrls.exit.save_dbgctl    = 1;

   vm_exit_ctrls.msr_store_addr.raw = (offset_t)&info->vm.cpu.vmc->msr_exit_store;
   vm_exit_ctrls.msr_load_addr.raw  = (offset_t)&info->vm.cpu.vmc->msr_exit_load;
}

static void vmx_vmcs_exec_controls_msr_init()
{
#ifndef CONFIG_MSR_PROXY
   vmx_deny_msr_rw(info->vm.cpu.vmc, IA32_MTRR_DEF_TYPE);
   vmx_deny_msr_rw(info->vm.cpu.vmc, IA32_EFER_MSR);

   /* XXX: modifying MTRRs while enabled must be intercepted */
   /* for(i=0 ; i<info->vm.mtrr_cap.vcnt ; i++) */
   /* { */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_PHYSBASE0+(i*2)); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_PHYSMASK0+(i*2)+1); */
   /* } */

   /* if(info->vm.mtrr_cap.fix) */
   /* { */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX64K_00000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX16K_80000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX16K_a0000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX4K_c0000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX4K_c8000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX4K_d0000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX4K_d8000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX4K_e0000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX4K_e8000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX4K_f0000); */
   /*    vmx_deny_msr_wr(info->vm.cpu.vmc, IA32_MTRR_FIX4K_f8000); */
   /* } */

   vm_exec_ctrls.proc.umsr = 1;
#endif

   vm_exec_ctrls.msr_bitmap.raw = (offset_t)info->vm.cpu.vmc->msr_bitmap;
}

static void vmx_vmcs_exec_controls_io_init()
{
   vm_exec_ctrls.proc.usio = 1;
   vm_exec_ctrls.io_bitmap_a.raw = (offset_t)info->vm.cpu.vmc->io_bitmap_a;
   vm_exec_ctrls.io_bitmap_b.raw = (offset_t)info->vm.cpu.vmc->io_bitmap_b;
}

static void vmx_vmcs_exec_controls_init()
{
   //vm_exec_ctrls.pin.preempt = 1;
   //vm_exec_ctrls.pin.nmi     = 1;

   vm_exec_ctrls.proc.tsc   = 1;
   vm_exec_ctrls.proc.cr3l  = 1;
   vm_exec_ctrls.proc.mwait = 0;
   vm_exec_ctrls.proc.mon   = 0;
   vm_exec_ctrls.proc.proc2 = 1;

   vm_exec_ctrls.proc2.ept    = 1;
   vm_exec_ctrls.proc2.vpid   = 1;
   vm_exec_ctrls.proc2.uguest = 1;
   vm_exec_ctrls.proc2.rdtscp = 1;

   vm_exec_ctrls.eptp.cache = VMX_EPT_MEM_TYPE_WB;
   vm_exec_ctrls.eptp.pwl   = 3;
   vm_exec_ctrls.eptp.addr  = page_nr(info->vm.cpu.pg[0].pml4);

   vm_exec_ctrls.vpid.raw   = 1;

   vm_exec_ctrls.excp_bitmap.raw = info->vm.cpu.dflt_excp;
   vm_exec_ctrls.pagefault_err_code_mask.raw = 0;
   vm_exec_ctrls.pagefault_err_code_match.raw = 0;

   info->vm.cr0_dft_mask.pe = 1;
   info->vm.cr0_dft_mask.cd = 1;
   info->vm.cr0_dft_mask.pg = 1;
   vm_exec_ctrls.cr0_mask.raw = info->vm.cr0_dft_mask.raw;

   info->vm.cr4_dft_mask.mce  = 1;
   info->vm.cr4_dft_mask.pae  = 1;
   info->vm.cr4_dft_mask.pse  = 1;
   info->vm.cr4_dft_mask.pge  = 1;
   info->vm.cr4_dft_mask.vmxe = 1;
   vm_exec_ctrls.cr4_mask.raw = info->vm.cr4_dft_mask.raw;

   vmx_vmcs_exec_controls_io_init();
   vmx_vmcs_exec_controls_msr_init();
}

static void vmx_vmcs_controls_init()
{
   vmx_vmcs_exec_controls_init();
   vmx_vmcs_exit_controls_init();
   vmx_vmcs_entry_controls_init();
}

void vmx_vmcs_init()
{
   vmx_vmcs_controls_init();
   vmx_vmcs_host_state_init();
   vmx_vmcs_guest_state_init();
}
