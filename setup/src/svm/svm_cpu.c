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
#include <svm_vmm.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void svm_cpu_features()
{
   amd_vmcr_msr_t vmcr;

   if(!svm_supported())
      panic("svm not supported");

   if(!svm_npt_supported())
      panic("svm npt feature not supported");

   rd_msr_vmcr(vmcr);
   if(vmcr.svm_dis && vmcr.lock)
      panic("svm feature BIOS-locked");
}

static void svm_cpu_skillz()
{
   info->vm.cpu.skillz.pg_2M = 1;
   info->vm.cpu.skillz.pg_1G = info->vmm.cpu.skillz.pg_1G;

   if(svm_flush_asid_supported())
   {
      info->vm.cpu.skillz.flush_tlb     = VMCB_TLB_CTL_FLUSH_GUEST;
      info->vm.cpu.skillz.flush_tlb_glb = VMCB_TLB_CTL_FLUSH_GUEST_ALL;
   }
   else
   {
      info->vm.cpu.skillz.flush_tlb     = VMCB_TLB_CTL_FLUSH_ALL;
      info->vm.cpu.skillz.flush_tlb_glb = VMCB_TLB_CTL_FLUSH_ALL;
   }
}

void svm_vm_cpu_skillz_init()
{
   svm_cpu_features();
   svm_cpu_skillz();
}
