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
#include <vmx_vmm.h>
#include <msr.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void vmx_cpu_features()
{
   feat_ctl_msr_t feat;

   if(!vmx_supported())
      panic("vmx not supported");

   rd_msr_feature_ctrl(feat);

   if(!feat.vmx && feat.lock)
      panic("vmx feature BIOS-locked: 0x%x", feat.raw);

   if(!feat.lock)
   {
      if(!feat.vmx)
	 feat.vmx = 1;

      feat.lock = 1;
      wr_msr_feature_ctrl(feat);
   }

   rd_msr_vmx_basic_info(info->vm.vmx_info);

   debug(VMX_CPU, "vmx revision %d\n", info->vm.vmx_info.revision_id);

   if(info->vm.vmx_info.true_f1)
   {
      rd_msr_vmx_true_pin_ctls(info->vm.vmx_fx_pin);
      rd_msr_vmx_true_proc_ctls(info->vm.vmx_fx_proc);
      rd_msr_vmx_true_exit_ctls(info->vm.vmx_fx_exit);
      rd_msr_vmx_true_entry_ctls(info->vm.vmx_fx_entry);
   }
   else
   {
      rd_msr_vmx_pin_ctls(info->vm.vmx_fx_pin);
      rd_msr_vmx_proc_ctls(info->vm.vmx_fx_proc);
      rd_msr_vmx_exit_ctls(info->vm.vmx_fx_exit);
      rd_msr_vmx_entry_ctls(info->vm.vmx_fx_entry);
   }

   if(!info->vm.vmx_fx_proc.proc2)
      panic("vmx missing features (secondary proc based) !");

   rd_msr_vmx_proc2_ctls(info->vm.vmx_fx_proc2);

   /* if(!vmx_allow_ept(info->vm.vmx_fx_proc2)) */
   if(!info->vm.vmx_fx_proc2.ept)
      panic("vmx ept not supported");

   if(!info->vm.vmx_fx_proc2.dt)
      panic("vmx desc table exiting not supported");

   if(!info->vm.vmx_fx_proc2.vpid)
      panic("vmx vpid not supported");

   if(!info->vm.vmx_fx_proc2.uguest)
      panic("vmx missing unrestricted guest\n");

   rd_msr_vmx_ept_cap(info->vm.vmx_ept_cap);

   if(!info->vm.vmx_ept_cap.wb)
      panic("vmx ept mem type only UC");

   if(!info->vm.vmx_ept_cap.pwl4)
      panic("unsupported page walk length");

   if(!info->vm.vmx_ept_cap.invvpid)
      panic("vmx missing invvpid\n");

   if(!info->vm.vmx_ept_cap.invept)
      panic("vmx missing invept\n");

   rd_msr_vmx_fixed_cr0(info->vm.vmx_fx_cr0);
   rd_msr_vmx_fixed_cr4(info->vm.vmx_fx_cr4);

   /* unrestricted guest */
   info->vm.vmx_fx_cr0.allow_0 &= ~(CR0_PG|CR0_PE);
   info->vm.vmx_fx_cr0.allow_1 |=  (CR0_PG|CR0_PE);
}

static void vmx_cpu_skillz()
{
   rd_msr_ia32_mtrr_def(info->vm.mtrr_def);
   rd_msr_ia32_mtrr_cap(info->vm.mtrr_cap);

   if(cpuid_ext_highest() < CPUID_MAX_ADDR)
      info->vm.max_paddr = (1ULL<<36) - 1;
   else
      info->vm.max_paddr = cpuid_max_paddr();

   info->vm.cpu.skillz.pg_2M = info->vm.vmx_ept_cap.pg_2m;
   info->vm.cpu.skillz.pg_1G = info->vm.vmx_ept_cap.pg_1g;

   if(info->vm.vmx_ept_cap.invvpid_s && info->vm.vmx_ept_cap.invvpid_r)
   {
      info->vm.cpu.skillz.flush_tlb     = VMCS_VPID_INV_SINGLE;
      info->vm.cpu.skillz.flush_tlb_glb = VMCS_VPID_INV_SINGLE_ALL;
   }
   else if(info->vm.vmx_ept_cap.invvpid_a)
   {
      info->vm.cpu.skillz.flush_tlb     = VMCS_VPID_INV_ALL;
      info->vm.cpu.skillz.flush_tlb_glb = VMCS_VPID_INV_ALL;
   }
   else
      panic("no valid invvpid type found");

   debug(VMX_CPU,
	 "vm 1GB pages support: %s\n"
	 "vm 2MB pages support: %s\n"
	 "vm max physical addr: 0x%X\n"
	 ,info->vm.cpu.skillz.pg_1G?"yes":"no"
	 ,info->vm.cpu.skillz.pg_2M?"yes":"no"
	 ,info->vm.max_paddr);
}

void vmx_vm_cpu_skillz_init()
{
   set_cr4(get_cr4()|CR4_OSXSAVE);

   vmx_cpu_features();
   vmx_cpu_skillz();
}
