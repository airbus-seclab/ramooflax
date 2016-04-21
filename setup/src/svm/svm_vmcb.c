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
#include <vm.h>
#include <svm_guest.h>
#include <info_data.h>

extern info_data_t *info;

void svm_vmcb_msr_controls_init()
{
#ifdef CONFIG_MSR_PROXY
   svm_deny_msr_rw_range(info->vm.cpu.vmc, 0, 0x2000-1);
   svm_deny_msr_rw_range(info->vm.cpu.vmc, 0xc0000000, 0xc0001fff-1);
   svm_deny_msr_rw_range(info->vm.cpu.vmc, 0xc0010000, 0xc0011fff-1);
#else
   svm_deny_msr_rw(info->vm.cpu.vmc, AMD_EFER_MSR);
#endif

   vm_ctrls.sys_insn_bitmap.intn = 1;
   vm_ctrls.sys_insn_bitmap.msr_insn = 1;
   vm_ctrls.msr_bitmap_addr.raw = (offset_t)info->vm.cpu.vmc->msr_bitmap;
}

static void svm_vmcb_io_controls_init()
{
   vm_ctrls.sys_insn_bitmap.io_insn = 1;
   vm_ctrls.io_bitmap_addr.raw = (offset_t)info->vm.cpu.vmc->io_bitmap;
}

static void svm_vmcb_controls_init()
{
   amd_svm_feat_t svm_feat;

   cpuid_amd_svm_features(svm_feat);
   if(svm_feat.asid_nr < 1)
      panic("no asid");

   if(!svm_feat.lbr)
      panic("no lbr virtualization");

   vm_ctrls.lbr.raw = 1ULL;

   vm_ctrls.exception_bitmap.raw = info->vm.cpu.dflt_excp;

   vm_ctrls.cr_read_bitmap  = VMCB_CR_R_BITMAP;
   vm_ctrls.cr_write_bitmap = VMCB_CR_W_BITMAP;

   vm_ctrls.sys_insn_bitmap.cr0_sel_write = 1;

   vm_ctrls.sys_insn_bitmap.cpuid      = 1;
   vm_ctrls.sys_insn_bitmap.task_sw    = 1;
   vm_ctrls.sys_insn_bitmap.freez      = 1;
   vm_ctrls.sys_insn_bitmap.shutdown   = 1;

   vm_ctrls.vmm_insn_bitmap.vmrun      = 1;
   vm_ctrls.vmm_insn_bitmap.vmmcall    = 1;
   vm_ctrls.vmm_insn_bitmap.vmload     = 1;
   vm_ctrls.vmm_insn_bitmap.vmsave     = 1;

   vm_ctrls.vmm_insn_bitmap.skinit     = 1;
   vm_ctrls.vmm_insn_bitmap.rdtscp     = 1;
   vm_ctrls.vmm_insn_bitmap.icebp      = 1;
   vm_ctrls.vmm_insn_bitmap.monitor    = 1;
   vm_ctrls.vmm_insn_bitmap.mwait      = 1;
   vm_ctrls.vmm_insn_bitmap.mwait_cond = 1;

   info->vm.asid_nr                 = svm_feat.asid_nr;
   vm_ctrls.sys_insn_bitmap.invlpga = 1;
   vm_ctrls.tlb_ctrl.guest_asid     = npg_get_default_paging()->asid;
   vm_ctrls.npt.raw                 = 1UL;
   vm_ctrls.ncr3.pml4.addr          = page_nr(npg_get_default_paging()->pml4);

   vm_ctrls.sys_insn_bitmap.intn    = 1;
   vm_ctrls.vmm_insn_bitmap.stgi    = 1;
   vm_ctrls.vmm_insn_bitmap.clgi    = 1;

   vm_ctrls.sys_insn_bitmap.init    = 1;
   vm_ctrls.sys_insn_bitmap.nmi     = 1;

   vm_ctrls.int_ctrl.v_ign_tpr      = 1;

   /*
   ** XXX: SMI not intercepted may disable interrupts
   ** (cf. revision guide for 10h familly)
   ** AMD tell to launch SMM in a guest
   ** to safely process i/o smi
   ** However, bios lock SMM regs
   ** and we are not able to properly
   ** handle all smi cases
   **
   ** So we don't intercept !
   */
}

void svm_vmcb_init()
{
   svm_vmcb_controls_init();
   svm_vmcb_io_controls_init();
   svm_vmcb_msr_controls_init();
   svm_vmcb_guest_state_init();
}
