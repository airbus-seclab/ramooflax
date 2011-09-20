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
#include <vmx_exit_cr.h>
#include <vmx_exit_fail.h>
#include <vmx_vmcs_acc.h>
#include <emulate.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

static int vmx_vmexit_resolve_lmsw(uint16_t val)
{
   raw64_t cr0;

   cr0.raw   = vm_exec_ctrls.cr0_read_shadow.raw & ~(0xfULL);
   cr0.wlow |= val & 0xf;

   return __resolve_cr_wr_with(0, (raw64_t*)&cr0.raw);
}

/* XXX: read cr0/cr4 will not trap !!! */
void __vmx_vmexit_cr_update_mask()
{
   if((info->vmm.ctrl.usr.cr_rd & (1<<0)) || (info->vmm.ctrl.usr.cr_wr & (1<<0)))
      vm_exec_ctrls.cr0_mask.raw = -1UL;
   else
      vm_exec_ctrls.cr0_mask.raw = info->vm.cr0_dft_mask.raw;

   vmcs_read(vm_exec_ctrls.proc);
   if((info->vmm.ctrl.usr.cr_rd & (1<<3)))
      vm_exec_ctrls.proc.cr3s = 1;
   else
      vm_exec_ctrls.proc.cr3s = 0;

   if((info->vmm.ctrl.usr.cr_rd & (1<<4)) || (info->vmm.ctrl.usr.cr_wr & (1<<4)))
      vm_exec_ctrls.cr4_mask.raw = -1UL;
   else
      vm_exec_ctrls.cr4_mask.raw = info->vm.cr4_dft_mask.raw;

   vmcs_dirty(vm_exec_ctrls.cr0_mask);
   vmcs_dirty(vm_exec_ctrls.proc);
   vmcs_dirty(vm_exec_ctrls.cr4_mask);
}

int vmx_vmexit_resolve_cr_access()
{
   vmcs_exit_info_cr_t *access;
   uint8_t             gpr;
   int                 rc;

   vmcs_read(vm_exit_info.qualification);
   access = &vm_exit_info.qualification.cr;
   gpr = GPR64_RAX - (access->gpr & GPR64_RAX);

   switch(access->type)
   {
   case VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_MOV_T_CR:
      rc = __resolve_cr(1, access->nr, gpr);
      break;
   case VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_MOV_F_CR:
      rc = __resolve_cr(0, access->nr, gpr);
      break;
   case VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_LMSW:
      if(access->lmsw_op)
	 return 0;
      rc = vmx_vmexit_resolve_lmsw(info->vm.cpu.gpr->raw[gpr].wlow);
      break;
   case VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_CLTS:
      rc = emulate_clts();
      break;
   default:
      return 0;
   }

   if(rc == CR_SUCCESS)
   {
      vmcs_read(vm_exit_info.insn_len);
      vm_update_rip(vm_exit_info.insn_len.raw);
      return 1;
   }

   return 0;
}
