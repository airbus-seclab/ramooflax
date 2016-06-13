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
#include <vmx_exit_db.h>
#include <db.h>
#include <vmx_vm.h>
#include <vmx_vmcs_acc.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/* Vol 3C Sections 26.6.3, 32.2, 25.5.2, 26.5.2
**
** no pending DB delivered after vm entry if:
** - vm entry is vectoring external int, nmi, hard excp, soft pvl excp
** - no mov ss and vectoring soft int or soft excp
** - vm entry is not vectoring, activity == shutdown or wait sipi
*/
static int vmx_db_pending_discarded()
{
   if(!vm_entry_ctrls.int_info.v)
   {
      vmcs_read(vm_state.activity);
      if(vm_state.activity.raw == VMX_VMCS_GUEST_ACTIVITY_STATE_SHUTDOWN ||
         vm_state.activity.raw == VMX_VMCS_GUEST_ACTIVITY_STATE_SIPI)
         return 1;
   }
   else
   {
      switch(vm_entry_ctrls.int_info.type)
      {
      case VMCS_EVT_INFO_TYPE_NMI:
      case VMCS_EVT_INFO_TYPE_HW_INT:
      case VMCS_EVT_INFO_TYPE_HW_EXCP:
      case VMCS_EVT_INFO_TYPE_PS_EXCP:
         return 1;

      case VMCS_EVT_INFO_TYPE_SW_INT:
      case VMCS_EVT_INFO_TYPE_SW_EXCP:
         vmcs_read(vm_state.interrupt);
         if(!vm_state.interrupt.mss)
            return 1;
      }
   }

   return 0;
}

/*
** if we filter #DB, vmx_exit_excp() will update dr6
** if we don't, the cpu will do it as normal
*/
static int vmx_db_check_pending_stp()
{
   if(!vm_state.rflags.tf)
      return VM_IGNORE;

   __rflags.tf = 0;
   __post_access(__rflags);

   return VM_DONE;
}

/*
** Hardware exec traps are checked before
** insn execution. But hardware data, i/o
** and single-step traps are checked after.
**
** If we emulated an insn, we may loose
** a #DB condition, so take care here.
**
** We do not inject #DB, we use pending db
*/
static int vmx_db_check_pending_any()
{
   if(!__vmexit_on_insn())
   {
#ifdef CONFIG_VMX_DB_DBG
      if(vm_state.rflags.tf)
      {
         vmcs_read(vm_state.activity);
         vmcs_read(vm_state.interrupt);
         debug(VMX_DB,
               "TF is set, pending #DB: be:%d bs:%d sti:%d mss:%d activity:0x%x\n"
               ,vm_state.dbg_excp.be, vm_state.dbg_excp.bs
               ,vm_state.interrupt.sti, vm_state.interrupt.mss
               ,vm_state.activity.raw);
      }
#endif
      return VM_IGNORE;
   }

   if(vmx_db_check_pending_stp() == VM_DONE)
   {
      debug(VMX_DB, "pending #DB: set stp\n");
      vm_state.dbg_excp.bs = 1;
      vmcs_dirty(vm_state.dbg_excp);
   }

   /* XXX: missing data/io */

   return VM_DONE;
}

void vmx_db_check_pending()
{
   vmcs_read(vm_state.dbg_excp);

   vmx_db_check_pending_any();

   if((vm_state.dbg_excp.be || vm_state.dbg_excp.bs) && vmx_db_pending_discarded())
      debug(VMX_DB, "pending #DB: lost one\n");
}

/*
** Vol. 3C-32.2 (Virtualization of System Resources)
*/
void vmx_check_dbgctl()
{
   vmcs_read(vm_state.activity);
   vmcs_read(vm_state.interrupt);

#ifdef CONFIG_VMX_DB_DBG
   vmcs_read(vm_state.dbg_excp);

   if(vm_state.dbg_excp.be || vm_state.dbg_excp.bs)
      debug(VMX_DB, "pending #DB: be:%d bs:%d 0x%X\n"
            ,vm_state.dbg_excp.be, vm_state.dbg_excp.bs
            ,vm_state.dbg_excp.raw);
#endif

   if(!vm_state.interrupt.sti && !vm_state.interrupt.mss &&
      vm_state.activity.raw != VMX_VMCS_GUEST_ACTIVITY_STATE_HALT)
      return;

#ifdef CONFIG_VMX_DB_DBG
   debug(VMX_DB, "pending #DB: sti:%d mss:%d activity:0x%x\n"
         ,vm_state.interrupt.sti, vm_state.interrupt.mss
         ,vm_state.activity.raw);
#endif

   vmcs_read(vm_state.ia32_dbgctl);
   vmcs_read(vm_state.dbg_excp);

   if(vm_state.rflags.tf && !vm_state.ia32_dbgctl.btf)
   {
      debug(VMX_DB, "pending #DB (sti/mss/hlt): set sstep\n");
      vm_state.dbg_excp.bs = 1;
   }
   else if(vm_state.dbg_excp.bs)
   {
      debug(VMX_DB, "pending #DB (sti/mss/hlt): clr sstep\n");
      vm_state.dbg_excp.bs = 0;
   }

   vmcs_dirty(vm_state.dbg_excp);
}

void vmx_db_show_pending()
{
#ifdef CONFIG_VMX_DB_DBG
   vmcs_read(vm_state.activity);
   vmcs_read(vm_state.interrupt);
   vmcs_read(vm_state.ia32_dbgctl);
   vmcs_read(vm_state.dbg_excp);

   debug(VMX_DB,
         "pending #DB: be:%d bs:%d raw:0x%X sti:%d mss:%d activity:0x%x btf:%d\n"
         ,vm_state.dbg_excp.be, vm_state.dbg_excp.bs
         ,vm_state.dbg_excp.raw
         ,vm_state.interrupt.sti, vm_state.interrupt.mss
         ,vm_state.activity.raw
         ,vm_state.ia32_dbgctl.btf);
#endif
}
