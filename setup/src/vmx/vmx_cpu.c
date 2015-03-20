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

#ifdef CONFIG_VMX_CPU_DBG
#include <vmx_show.h>
#endif

extern info_data_t *info;

static void vmx_cpu_ept_feat()
{
   rd_msr_vmx_ept_cap(info->vm.vmx_ept_cap);

#ifdef CONFIG_VMX_CPU_DBG
   vmx_show_ept_cap();
#endif

   if(!info->vm.vmx_ept_cap.wb)
      panic("vmx ept mem type only UC");

   if(!info->vm.vmx_ept_cap.pwl4)
      panic("unsupported page walk length");

#ifdef CONFIG_VMX_FEAT_VPID
   if(!info->vm.vmx_ept_cap.invvpid)
      panic("vmx missing invvpid\n");
#endif

   if(!info->vm.vmx_ept_cap.invept)
      panic("vmx missing invept\n");
}

static void vmx_cpu_proc2_feat()
{
   rd_msr_vmx_proc2_ctls(info->vm.vmx_fx_proc2);

#ifdef CONFIG_VMX_CPU_DBG
   vmx_show_fixed_proc2_ctls();
#endif

   if(!info->vm.vmx_fx_proc2.allow_1.uguest)
      panic("vmx missing unrestricted guest\n");

#ifdef CONFIG_VMX_FEAT_EXIT_DT
   if(!info->vm.vmx_fx_proc2.allow_1.dt)
      panic("vmx desc table exiting not supported");
#endif

#ifdef CONFIG_VMX_FEAT_VPID
   if(!info->vm.vmx_fx_proc2.allow_1.vpid)
      panic("vmx vpid not supported");
#endif

   if(!info->vm.vmx_fx_proc2.allow_1.ept)
      panic("vmx ept not supported");

   vmx_cpu_ept_feat();
}

static void vmx_cpu_proc_feat()
{
   if(!info->vm.vmx_fx_proc.allow_1.proc2)
      panic("vmx missing secondary proc based !");

   vmx_cpu_proc2_feat();
}

static void vmx_cpu_basic_info()
{
   rd_msr_vmx_basic_info(info->vm.vmx_info);
   rd_msr_vmx_misc_data(info->vm.vmx_misc);

   if(info->vm.vmx_info.true_f1)
   {
      rd_msr_vmx_true_pin_ctls(info->vm.vmx_fx_pin);
      rd_msr_vmx_true_proc_ctls(info->vm.vmx_fx_proc);
      rd_msr_vmx_true_entry_ctls(info->vm.vmx_fx_entry);
      rd_msr_vmx_true_exit_ctls(info->vm.vmx_fx_exit);
   }
   else
   {
      rd_msr_vmx_pin_ctls(info->vm.vmx_fx_pin);
      rd_msr_vmx_proc_ctls(info->vm.vmx_fx_proc);
      rd_msr_vmx_entry_ctls(info->vm.vmx_fx_entry);
      rd_msr_vmx_exit_ctls(info->vm.vmx_fx_exit);
   }

#ifdef CONFIG_VMX_CPU_DBG
   vmx_show_basic_info();
   vmx_show_misc_data();
   vmx_show_fixed_pin_ctls();
   vmx_show_fixed_proc_ctls();
   vmx_show_fixed_entry_ctls();
   vmx_show_fixed_exit_ctls();
#endif

   /* should be 1 if uguest available, and as we need uguest */
   if(!info->vm.vmx_misc.lma)
      panic("vmx misc ia32e/lma missing");

#ifdef CONFIG_VMX_FEAT_EXIT_EXT_IO
   if(!info->vm.vmx_info.io_insn)
      panic("vmx ins/outs info not given on VM-exit");
#endif
}

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

   vmx_cpu_basic_info();
   vmx_cpu_proc_feat();

   rd_msr_vmx_fixed_cr0(info->vm.vmx_fx_cr0);
   rd_msr_vmx_fixed_cr4(info->vm.vmx_fx_cr4);

   /* unrestricted guest */
   info->vm.vmx_fx_cr0.fixed_1.raw &= ~(CR0_PG|CR0_PE);
   info->vm.vmx_fx_cr0.allow_1.raw |=  (CR0_PG|CR0_PE);
}

void vmx_cpu_max_addr()
{
   intel_max_addr_sz_t max;

   cpuid_max_addr(max.raw);

   if(info->vm.vmx_info.pwidth32)
      panic("VMCS pointers physical width limited to 32 bits\n");

   info->vmm.cpu.skillz.paddr_sz = max.paddr_sz;
   info->vmm.cpu.max_paddr = (1ULL<<max.paddr_sz) - 1;

   info->vmm.cpu.skillz.vaddr_sz = max.vaddr_sz;
   info->vmm.cpu.max_vaddr = (1ULL<<max.vaddr_sz) - 1;

   info->vm.cpu.skillz.paddr_sz = info->vmm.cpu.skillz.paddr_sz;
   info->vm.cpu.max_paddr = info->vmm.cpu.max_paddr;

   info->vm.cpu.skillz.vaddr_sz = info->vmm.cpu.skillz.vaddr_sz;
   info->vm.cpu.max_vaddr = info->vmm.cpu.max_vaddr;
}

static void vmx_cpu_skillz()
{
   rd_msr_ia32_mtrr_def(info->vm.mtrr_def);
   rd_msr_ia32_mtrr_cap(info->vm.mtrr_cap);

   info->vm.cpu.skillz.pg_2M = info->vm.vmx_ept_cap.pg_2m;
   info->vm.cpu.skillz.pg_1G = info->vm.vmx_ept_cap.pg_1g;

#ifdef CONFIG_VMX_FEAT_VPID
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
#endif

   debug(VMX_CPU,
	 "\n- vm cpu features\n"
	 "1GB pages support   : %s\n"
	 "2MB pages support   : %s\n"
	 "max physical addr   : 0x%X\n"
	 "max linear addr     : 0x%X\n"
	 "mtrr variable count : %d\n"
	 ,info->vm.cpu.skillz.pg_1G?"yes":"no"
	 ,info->vm.cpu.skillz.pg_2M?"yes":"no"
	 ,info->vm.cpu.max_paddr, info->vm.cpu.max_vaddr
	 ,info->vm.mtrr_cap.vcnt);
}

void vmx_vm_cpu_skillz_init()
{
   vmx_cpu_features();
   vmx_cpu_skillz();
}
