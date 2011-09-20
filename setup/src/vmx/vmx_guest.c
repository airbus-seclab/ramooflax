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
#include <vmx_guest.h>
#include <vmx_msr.h>
#include <intr.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void vmx_vmcs_guest_nonregister_state_init()
{
   vmx_misc_data_msr_t misc_data;

   rd_msr_vmx_misc_data(misc_data);
   misc_data.eax = (misc_data.eax>>6) & 0x3;
   vm_state.activity_state.raw = VMX_VMCS_GUEST_ACTIVITY_STATE_ACTIVE & misc_data.eax;

   vm_state.vmcs_link_ptr.raw = -1ULL;
   //vm_state.preempt_timer.raw = 0x200000;
}

static void vmx_vmcs_guest_register_state_segments_init()
{
   vm_state.idtr.limit.wlow = BIOS_MISC_INTERRUPT*sizeof(ivt_e_t) - 1;

   vm_state.ss.selector.raw = RM_BASE_SS;
   vm_state.ss.base.low = (vm_state.ss.selector.raw)<<4;

   vm_state.cs.limit.raw = 0xffff;
   vm_state.ss.limit.raw = 0xffff;
   vm_state.ds.limit.raw = 0xffff;
   vm_state.es.limit.raw = 0xffff;
   vm_state.fs.limit.raw = 0xffff;
   vm_state.gs.limit.raw = 0xffff;

   vm_state.cs.attributes.raw = VMX_CODE_16_R0_CO_ATTR;
   vm_state.ss.attributes.raw = VMX_DATA_16_R0_ATTR;

   vm_state.ds.attributes.raw = VMX_DATA_16_R3_ATTR;
   vm_state.es.attributes.raw = VMX_DATA_16_R3_ATTR;
   vm_state.fs.attributes.raw = VMX_DATA_16_R3_ATTR;
   vm_state.gs.attributes.raw = VMX_DATA_16_R3_ATTR;

   vm_state.tr.attributes.raw = VMX_TSS_32_ATTR;
   vm_state.ldtr.attributes.u = 1;
}

static void vmx_vmcs_guest_register_state_init()
{
   /* ensure to catch #MC */
   info->vm.vmx_fx_cr4.allow_0 |= (CR4_MCE);
   info->vm.vmx_fx_cr4.allow_1 |= (CR4_MCE);

   rd_msr_ia32_dbg_ctl(vm_state.ia32_dbgctl);
   rd_msr_pat(vm_state.ia32_pat);

   vm_state.rflags.r1 = 1;
   vm_state.rflags.IF = 1;

   vm_state.rsp.low = RM_BASE_SP;
   vm_state.rip.low = VM_ENTRY_POINT;
}

void vmx_vmcs_guest_state_init()
{
   vmx_vmcs_guest_register_state_init();
   vmx_vmcs_guest_register_state_segments_init();
   vmx_vmcs_guest_nonregister_state_init();
}
