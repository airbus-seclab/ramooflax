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
#include <vmx_exit_gp.h>
#include <vmx_exit_int.h>
#include <vmx_vm.h>
#include <vmx_vmcs_acc.h>
#include <emulate.h>
#include <emulate_int.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** In real-mode, exception #13 is not #GP
** it's segment overrun.
**
** On the 8086, if sequential execution of instructions proceeds past
** offset 65535, the processor fetches the next instruction byte from
** offset 0 of the same segment. On the 80386, the processor raises
** exception 13 in such a case.
**
*/
int vmx_vmexit_resolve_rmode_gp()
{
   vmcs_read(vm_exit_info.idt_info);

   /* #GP are related to IDT events (idt limit) */
   if(!vm_exit_info.idt_info.v)
   {
      debug(VMX_EXCP_GP, "rmode #GP: not related to idt\n");
      return VM_FAIL;
   }

   if(vm_exit_info.idt_info.type == VMCS_EVT_INFO_TYPE_SW_INT)
      return emulate();

   if(vm_exit_info.idt_info.type == VMCS_EVT_INFO_TYPE_HW_INT)
      return emulate_hard_interrupt(vm_exit_info.idt_info.vector);

   debug(VMX_EXCP_GP, "rmode #GP: invalid type (%d)\n", vm_exit_info.idt_info.type);
   return VM_FAIL;
}
