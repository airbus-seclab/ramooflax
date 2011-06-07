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
#include <vmx_exit_int.h>
#include <vmx_vmcs_acc.h>
#include <int.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

void __vmx_vmexit_setup_interrupt_window_exiting(uint8_t enable, uint8_t irq)
{
   vmx_procbased_ctls_msr_t proc_msr;
   vmcs_exec_ctl_t          *ctls = exec_ctrls(info);

   vmcs_read( ctls->proc );

   if( (enable && ! ctls->proc.iwe ) || (! enable && ctls->proc.iwe) )
   {
      ctls->proc.iwe = enable;

      debug( VMX_INT, "i-w-e %s\n", enable?"on":"off" );

      read_msr_vmx_procbased_ctls( proc_msr );
      ctls->proc.raw &= proc_msr.allowed_1_settings;
      ctls->proc.raw |= proc_msr.allowed_0_settings;

      vmcs_dirty( ctls->proc );

      info->vm.last_pending = irq;
   }
}

void __vmx_vmexit_inject_interrupt(uint8_t vector)
{
   vmcs_guest_t     *state = guest_state(info);
   vmcs_entry_ctl_t *ctrls = entry_ctrls(info);

   ctrls->int_info.raw    = 0;
   ctrls->int_info.vector = vector;
   ctrls->int_info.type   = VMCS_IDT_INFORMATION_TYPE_HW_INT;
   ctrls->int_info.v      = 1;

   vmcs_dirty( ctrls->int_info );

   /*
   ** vm-entry checks
   */
   if( ! state->rflags.IF )
   {
      state->rflags.IF = 1;
      vmcs_dirty( state->rflags );
   }

   vmcs_read( state->int_state );

   if( state->int_state.sti || state->int_state.mss )
   {
      state->int_state.sti = 0;
      state->int_state.mss = 0;
      vmcs_dirty( state->int_state );

      debug( VMX_INT, "inject IRQ forced (interrupt shadow)\n" );
   }
}

