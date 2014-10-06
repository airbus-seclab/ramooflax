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
#include <vmx_insn.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

char vmx_vmlaunch_failure_fmt[] = "failed (%d)\n";
char vmx_vmlaunch_failure_fnm[] = "vmlaunch";

void vmx_vmm_init()
{
   cr0_reg_t cr0;
   cr4_reg_t cr4;
   raw64_t   vmcs_addr;

   info->vm.cpu.vmc->vmm_cpu_vmcs.revision_id = info->vm.vmx_info.revision_id;

   cr0.raw = get_cr0();
   vmx_set_fixed(cr0.low, info->vm.vmx_fx_cr0);
   set_cr0(cr0);

   cr4.raw = get_cr4()|CR4_OSXSAVE;
   vmx_set_fixed(cr4.low, info->vm.vmx_fx_cr4);
   set_cr4(cr4);

   enable_vmx();

   vmcs_addr.raw = (offset_t)&info->vm.cpu.vmc->vmm_cpu_vmcs;

   if(!vmx_vmxon(&vmcs_addr.raw))
      panic("failed to enter vmx root operations");
}
