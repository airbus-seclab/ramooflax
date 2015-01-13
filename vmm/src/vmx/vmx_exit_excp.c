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
#include <vmx_exit_excp.h>
#include <vmx_exit_gp.h>
#include <vmx_vmcs_acc.h>
#include <excp.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

void __vmx_clear_event_injection()
{
   if(!vm_entry_ctrls.int_info.v)
      return;

   if(vm_entry_ctrls.int_info.type == VMCS_IDT_INFO_TYPE_HW_EXCP &&
      vm_entry_ctrls.int_info.vector == PF_EXCP)
      vmcs_clear(vm_state.cr2);

   vm_entry_ctrls.int_info.v = 0;
   vmcs_dirty(vm_entry_ctrls.int_info);
}

int __vmx_vmexit_inject_exception(uint32_t vector, uint32_t error, uint64_t cr2)
{
   __vmx_prepare_event_injection(vm_entry_ctrls.int_info,
				 VMCS_IDT_INFO_TYPE_HW_EXCP,
				 vector);
   switch(vector)
   {
   case PF_EXCP:
      vm_state.cr2.raw = cr2;
      vmcs_dirty(vm_state.cr2);
   case DF_EXCP:
   case TS_EXCP:
   case NP_EXCP:
   case SS_EXCP:
   case GP_EXCP:
   case AC_EXCP:
      vm_entry_ctrls.int_info.dec = 1;
      vm_entry_ctrls.err_code.raw = error;
      vmcs_dirty(vm_entry_ctrls.err_code);
      break;

   default:
      break;
   }

   debug(VMX_EXCP, "inject exception #%d err 0x%x\n", vector, error);
   return 1;
}

int vmx_vmexit_resolve_excp()
{
   vmcs_read(vm_exit_info.int_info);
   vmcs_read(vm_exit_info.int_err_code);
   vmcs_read(vm_exit_info.qualification);

   switch(vm_exit_info.int_info.vector)
   {
   case GP_EXCP:
      if(__rmode())
	 return vmx_vmexit_resolve_gp();
      break;
   case DB_EXCP:
      vm_state.dr6.wlow = vm_exit_info.qualification.wlow;
      vmcs_set_read(vm_state.dr6);
      break;
   case MC_EXCP:
      vmm_excp_mce();
      break;
   }

   return resolve_exception();
}
