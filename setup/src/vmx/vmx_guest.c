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
#include <vmx_vmcs.h>
#include <vmx_msr.h>
#include <asmutils.h>
#include <info_data.h>

void vmx_vmcs_guest_state_init(info_data_t *info)
{
   vmx_vmcs_guest_register_state_init( info );
   vmx_vmcs_guest_register_state_segments_init( info );
   vmx_vmcs_guest_nonregister_state_init( info );
}

void vmx_vmcs_guest_register_state_init(info_data_t *info)
{
   cr0_reg_t     cr0_f0, cr0_f1;
   cr4_reg_t     cr4_f0, cr4_f1;
   vmcs_guest_t  *state;

   state = guest_state(info);

   //state->idtr.limit.wlow = 0x15*sizeof(ivt_e_t) - 1;

   read_msr_vmx_cr0_fixed0( cr0_f0 );
   read_msr_vmx_cr0_fixed1( cr0_f1 );

   read_msr_vmx_cr4_fixed0( cr4_f0 );
   read_msr_vmx_cr4_fixed1( cr4_f1 );

   state->cr0.low = ((CR0_ET|CR0_PE|CR0_PG) & cr0_f1.low) | cr0_f0.low;
   state->cr3.low = (uint32_t)info->vm.cpu.pg->rm_pgd;
   state->cr4.low = (CR4_PSE & cr4_f1.low) | cr4_f0.low;

   read_msr_ia32_debugctl( state->ia32_debugctl_msr );

   state->rflags.r1 = 1;
   state->rflags.IF = 1;

   state->rip.low = VM_ENTRY_POINT;
   state->rsp.low = RM_BASE_SP;
}

void vmx_vmcs_guest_register_state_segments_init(info_data_t *info)
{
   vmcs_guest_t *state = guest_state(info);

   state->ss.selector.raw  = RM_BASE_SS;
   state->ss.base_addr.low = (state->ss.selector.raw)<<4;

   state->cs.limit.raw = 0xffff;
   state->ss.limit.raw = 0xffff;
   state->ds.limit.raw = 0xffff;
   state->es.limit.raw = 0xffff;
   state->fs.limit.raw = 0xffff;
   state->gs.limit.raw = 0xffff;

   state->cs.attributes.raw = VMX_CODE_16_R0_CO_ATTR;
   state->ss.attributes.raw = VMX_DATA_16_R0_ATTR;

   state->ds.attributes.raw = VMX_DATA_16_R3_ATTR;
   state->es.attributes.raw = VMX_DATA_16_R3_ATTR;
   state->fs.attributes.raw = VMX_DATA_16_R3_ATTR;
   state->gs.attributes.raw = VMX_DATA_16_R3_ATTR;

   state->tr.attributes.raw = VMX_TSS_32_ATTR;
   state->ldtr.attributes.unuse = 1;
}

void vmx_vmcs_guest_nonregister_state_init(info_data_t *info)
{
   vmx_misc_data_msr_t misc_data;
   vmcs_guest_t        *state;

   state = guest_state(info);

   read_msr_vmx_misc_data( misc_data );
   misc_data.eax = (misc_data.eax>>6) & 0x3;

   state->activity_state.raw = VMX_VMCS_GUEST_ACTIVITY_STATE_ACTIVE & misc_data.eax;

   state->vmcs_link_ptr.raw = -1ULL;
}

