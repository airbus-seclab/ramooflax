/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#include <vmx_exit_db.h>
#include <db.h>
#include <vmx_vm.h>
#include <vmx_vmcs_acc.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

void vmx_check_pending_db()
{
   /*
   ** XXX: cf. Vol 3C. 26.6.3, 32.2, 25.5.2, 26.5.2
   ** - pending #DB
   ** - pending MTF
   **
   ** we should not exactly follow manual
   ** in case of gdbstub wanted #DB
   */
   db_check_pending();

   /* prevent invalid guest state */
   vmx_check_dbgctl();
}

/*
** Vol. 3C-32.2 (Virtualization of System Resources)
*/
void vmx_check_dbgctl()
{
   vmcs_read(vm_state.activity);
   vmcs_read(vm_state.dbg_excp);
   vmcs_read(vm_state.interrupt);
   vmcs_read(vm_state.ia32_dbgctl);

   if(!vm_state.interrupt.sti && !vm_state.interrupt.mss &&
      vm_state.activity.raw != VMX_VMCS_GUEST_ACTIVITY_STATE_HALT)
      return;

   if(vm_state.rflags.tf && !vm_state.ia32_dbgctl.btf)
   {
      debug(VMX_DB, "setting pending #DB for sstep\n");
      vm_state.dbg_excp.bs = 1;
   }

   if(!vm_state.rflags.tf || vm_state.ia32_dbgctl.btf)
      vm_state.dbg_excp.bs = 0;

   vmcs_dirty(vm_state.dbg_excp);
}
