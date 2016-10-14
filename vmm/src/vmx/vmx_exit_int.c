/*
** Copyright (C) 2016 Airbus Group, stephane duverger <stephane.duverger@airbus.com>
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
#include <vmx_exit_int.h>
#include <intr.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;
extern irq_msg_t    irq_msg;

void __vmx_vmexit_setup_interrupt_window_exiting(uint8_t enable, uint8_t vector)
{
   vmcs_read(vm_exec_ctrls.proc);

   if(enable ^ vm_exec_ctrls.proc.iwe)
   {
      vm_exec_ctrls.proc.iwe = enable;
      vmcs_dirty(vm_exec_ctrls.proc);
      irq_msg.pending = vector;
      debug(VMX_INT, "interrupt windows exiting: %s\n", enable ? "on":"off");
   }
}

void __vmx_vmexit_inject_interrupt(uint8_t vector)
{
   __vmx_prepare_event_injection(vm_entry_ctrls.int_info,
                                 VMCS_EVT_INFO_TYPE_HW_INT,
                                 vector);

   /* vm-entry checks  */
   if(!vm_state.rflags.IF)
   {
      vm_state.rflags.IF = 1;
      vmcs_dirty(vm_state.rflags);
   }

   vmcs_read(vm_state.interrupt);

   if(vm_state.interrupt.sti || vm_state.interrupt.mss)
   {
      vm_state.interrupt.sti = 0;
      vm_state.interrupt.mss = 0;
      vmcs_dirty(vm_state.interrupt);
      debug(VMX_INT, "inject IRQ %d forced (interrupt shadow)\n", vector);
   }
}

/*
** Interrupts just enabled inside the VM
** check if we need to inject something
** and then disable IWE
*/
int vmx_vmexit_resolve_iwe()
{
   __vmx_vmexit_inject_interrupt(irq_msg.pending);
   __vmx_vmexit_setup_interrupt_window_exiting(0, 0xff);

   return VM_DONE;
}

/*
** vmexit controls acknowlegde interrupt on exit
** so it is safe to retrieve the vector here
*/
int vmx_vmexit_resolve_intr()
{
   vmcs_read(vm_exit_info.int_info);
   irq_msg.vector = vm_exit_info.int_info.vector;

   return resolve_intr();
}
