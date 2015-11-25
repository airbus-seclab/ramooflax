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
#include <vmx_vm.h>
#include <vmx_vmcs.h>
#include <vmx_msr.h>
#include <vmx_insn.h>
#include <intr.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

void vmx_vm_init()
{
   vmx_insn_err_t vmx_err;
   raw64_t        vmcs_addr;

   info->vm.cpu.vmc->vm_cpu_vmcs.revision_id = info->vm.vmx_info.revision_id;
   info->vm.vmcs.revision_id = info->vm.vmx_info.revision_id;

   info->vm.dr_shadow[4].raw = 0xffff0ff0;
   info->vm.dr_shadow[5].raw = 0x400;
   info->vm.idt_limit_rmode  = BIOS_MISC_INTERRUPT*sizeof(ivt_e_t) - 1;

   vmcs_addr.raw = (offset_t)&info->vm.cpu.vmc->vm_cpu_vmcs;

   if(!vmx_vmclear(&vmx_err, &vmcs_addr.raw))
      panic("vmclear failed (%d) !\n", vmx_err.raw);

   if(!vmx_vmload(&vmx_err, &vmcs_addr.raw))
      panic("vmload failed (%d) !\n", vmx_err.raw);

   vmx_vmcs_init(info);
   vmx_vmcs_encode(info);
   vmx_vmcs_commit(info);
}
