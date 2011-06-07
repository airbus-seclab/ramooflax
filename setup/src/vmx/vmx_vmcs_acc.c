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
#include <vmx_vmcs_acc.h>
#include <vmx_insn.h>
#include <vmx_vmm.h>
#include <info_data.h>

void __vmcs_read(raw64_t *value, vmcs_field_access_t *access)
{
   if( access->enc.fwidth == VMCS_FIELD_ENC_FIELD_WIDTH_16 )
   {
      raw32_t tmp;
      vmx_vmread( "low16", &tmp.raw, access->enc.raw );
      value->wlow = tmp.wlow;
   }
   else
   {
      vmx_vmread( "low32", &value->low, access->enc.raw );

      if( access->enc.fwidth == VMCS_FIELD_ENC_FIELD_WIDTH_64 )
	 vmx_vmread( "high32", &value->high, access->enc.raw+1 );
   }

   access->r = 1;
}

void __vmcs_flush(raw64_t *value, vmcs_field_access_t *access)
{
   vmx_vmwrite( "low32", value->low, access->enc.raw );

   if( access->enc.fwidth == VMCS_FIELD_ENC_FIELD_WIDTH_64 )
      vmx_vmwrite( "high32", value->high, access->enc.raw+1 );

   access->w = 0;
}

void vmx_vmcs_commit(info_data_t *info)
{
   vmcs_exec_ctl_t   *exec_ctl  = exec_ctrls(info);
   vmcs_exit_ctl_t   *exit_ctl  = exit_ctrls(info);
   vmcs_entry_ctl_t  *entry_ctl = entry_ctrls(info);
   vmcs_host_t       *h_state   = host_state(info);
   vmcs_guest_t      *g_state   = guest_state(info);
   vmcs_exit_info_t  *exit_info = exit_info(info);

   /* execution controls */
   vmcs_flush( exec_ctl->pin );
   vmcs_flush( exec_ctl->proc );
   vmcs_flush( exec_ctl->excp_bitmap );
   vmcs_flush( exec_ctl->pagefault_err_code_mask );
   vmcs_flush( exec_ctl->pagefault_err_code_match );
   vmcs_flush( exec_ctl->io_bitmap_a );
   vmcs_flush( exec_ctl->io_bitmap_b );
   vmcs_flush( exec_ctl->tsc_offset );
   vmcs_flush( exec_ctl->cr0_mask );
   vmcs_flush( exec_ctl->cr4_mask );
   vmcs_flush( exec_ctl->cr0_read_shadow );
   vmcs_flush( exec_ctl->cr4_read_shadow );
   vmcs_flush( exec_ctl->cr3_target_0 );
   vmcs_flush( exec_ctl->cr3_target_1 );
   vmcs_flush( exec_ctl->cr3_target_2 );
   vmcs_flush( exec_ctl->cr3_target_3 );
   vmcs_flush( exec_ctl->cr3_target_count );
   vmcs_flush( exec_ctl->v_apic_page );
   vmcs_flush( exec_ctl->tpr_threshold );
   vmcs_flush( exec_ctl->msr_bitmap );
   vmcs_flush( exec_ctl->executive_vmcs_ptr );

   /* exit controls */
   vmcs_flush( exit_ctl->exit );
   vmcs_flush( exit_ctl->msr_store_count );
   vmcs_flush( exit_ctl->msr_store_addr );
   vmcs_flush( exit_ctl->msr_load_count );
   vmcs_flush( exit_ctl->msr_load_addr );

   /* entry controls */
   vmcs_flush( entry_ctl->entry );
   vmcs_flush( entry_ctl->msr_load_count );
   vmcs_flush( entry_ctl->msr_load_addr );
   vmcs_flush( entry_ctl->int_info );
   vmcs_flush( entry_ctl->err_code );
   vmcs_flush( entry_ctl->insn_len );

   /* host state */
   vmcs_flush( h_state->cr0 );
   vmcs_flush( h_state->cr3 );
   vmcs_flush( h_state->cr4 );
   vmcs_flush( h_state->rsp );
   vmcs_flush( h_state->rip );
   vmcs_flush( h_state->cs );
   vmcs_flush( h_state->ss );
   vmcs_flush( h_state->ds );
   vmcs_flush( h_state->es );
   vmcs_flush( h_state->fs );
   vmcs_flush( h_state->gs );
   vmcs_flush( h_state->tr );
   vmcs_flush( h_state->fs_base_addr );
   vmcs_flush( h_state->gs_base_addr );
   vmcs_flush( h_state->tr_base_addr );
   vmcs_flush( h_state->gdtr_base_addr );
   vmcs_flush( h_state->idtr_base_addr );
   vmcs_flush( h_state->ia32_sysenter_cs_msr );
   vmcs_flush( h_state->ia32_sysenter_esp_msr );
   vmcs_flush( h_state->ia32_sysenter_eip_msr );

   /* guest state */
   vmcs_flush( g_state->cr0 );
   vmcs_flush( g_state->cr3 );
   vmcs_flush( g_state->cr4 );
   vmcs_flush( g_state->dr7 );
   vmcs_flush( g_state->rsp );
   vmcs_flush( g_state->rip );
   vmcs_flush( g_state->rflags );
   vmcs_flush( g_state->cs.selector );
   vmcs_flush( g_state->ss.selector );
   vmcs_flush( g_state->ds.selector );
   vmcs_flush( g_state->es.selector );
   vmcs_flush( g_state->fs.selector );
   vmcs_flush( g_state->gs.selector );
   vmcs_flush( g_state->tr.selector );
   vmcs_flush( g_state->ldtr.selector );
   vmcs_flush( g_state->cs.base_addr );
   vmcs_flush( g_state->ss.base_addr );
   vmcs_flush( g_state->ds.base_addr );
   vmcs_flush( g_state->es.base_addr );
   vmcs_flush( g_state->fs.base_addr );
   vmcs_flush( g_state->gs.base_addr );
   vmcs_flush( g_state->tr.base_addr );
   vmcs_flush( g_state->ldtr.base_addr );
   vmcs_flush( g_state->cs.limit );
   vmcs_flush( g_state->ss.limit );
   vmcs_flush( g_state->ds.limit );
   vmcs_flush( g_state->es.limit );
   vmcs_flush( g_state->fs.limit );
   vmcs_flush( g_state->gs.limit );
   vmcs_flush( g_state->tr.limit );
   vmcs_flush( g_state->ldtr.limit );
   vmcs_flush( g_state->cs.attributes );
   vmcs_flush( g_state->ss.attributes );
   vmcs_flush( g_state->ds.attributes );
   vmcs_flush( g_state->es.attributes );
   vmcs_flush( g_state->fs.attributes );
   vmcs_flush( g_state->gs.attributes );
   vmcs_flush( g_state->tr.attributes );
   vmcs_flush( g_state->ldtr.attributes );
   vmcs_flush( g_state->gdtr.base_addr );
   vmcs_flush( g_state->gdtr.limit );
   vmcs_flush( g_state->idtr.base_addr );
   vmcs_flush( g_state->idtr.limit );
   vmcs_flush( g_state->ia32_debugctl_msr );
   vmcs_flush( g_state->ia32_sysenter_cs_msr );
   vmcs_flush( g_state->ia32_sysenter_esp_msr );
   vmcs_flush( g_state->ia32_sysenter_eip_msr );
   vmcs_flush( g_state->smbase );
   vmcs_flush( g_state->activity_state );
   vmcs_flush( g_state->int_state );
   vmcs_flush( g_state->dbg_excp );
   vmcs_flush( g_state->vmcs_link_ptr );

   /* exit info */
   exit_info->reason.__vmcs_access.r = 0;
   exit_info->qualification.__vmcs_access.r = 0;
   exit_info->int_info.__vmcs_access.r = 0;
   exit_info->int_err_code.__vmcs_access.r = 0;
   exit_info->idt_info.__vmcs_access.r = 0;
   exit_info->idt_err_code.__vmcs_access.r = 0;
   exit_info->vmexit_insn_len.__vmcs_access.r = 0;
   exit_info->guest_addr.__vmcs_access.r = 0;
   exit_info->vmx_insn_info.__vmcs_access.r = 0;
   exit_info->io_rcx.__vmcs_access.r = 0;
   exit_info->io_rsi.__vmcs_access.r = 0;
   exit_info->io_rdi.__vmcs_access.r = 0;
   exit_info->io_rip.__vmcs_access.r = 0;
   exit_info->vm_insn_err.__vmcs_access.r = 0;

}

void vmx_vmcs_collect(info_data_t *info)
{
   vmcs_exec_ctl_t   *exec_ctl  = exec_ctrls(info);
   vmcs_exit_ctl_t   *exit_ctl  = exit_ctrls(info);
   vmcs_entry_ctl_t  *entry_ctl = entry_ctrls(info);
   vmcs_host_t       *h_state   = host_state(info);
   vmcs_guest_t      *g_state   = guest_state(info);
   vmcs_exit_info_t  *exit_info = exit_info(info);

   /* execution controls */
   vmcs_read( exec_ctl->pin );
   vmcs_read( exec_ctl->proc );
   vmcs_read( exec_ctl->excp_bitmap );
   vmcs_read( exec_ctl->pagefault_err_code_mask );
   vmcs_read( exec_ctl->pagefault_err_code_match );
   vmcs_read( exec_ctl->io_bitmap_a );
   vmcs_read( exec_ctl->io_bitmap_b );
   vmcs_read( exec_ctl->tsc_offset );
   vmcs_read( exec_ctl->cr0_mask );
   vmcs_read( exec_ctl->cr4_mask );
   vmcs_read( exec_ctl->cr0_read_shadow );
   vmcs_read( exec_ctl->cr4_read_shadow );
   vmcs_read( exec_ctl->cr3_target_0 );
   vmcs_read( exec_ctl->cr3_target_1 );
   vmcs_read( exec_ctl->cr3_target_2 );
   vmcs_read( exec_ctl->cr3_target_3 );
   vmcs_read( exec_ctl->cr3_target_count );
   vmcs_read( exec_ctl->v_apic_page );
   vmcs_read( exec_ctl->tpr_threshold );
   vmcs_read( exec_ctl->msr_bitmap );
   vmcs_read( exec_ctl->executive_vmcs_ptr );

   /* exit controls */
   vmcs_read( exit_ctl->exit );
   vmcs_read( exit_ctl->msr_store_count );
   vmcs_read( exit_ctl->msr_store_addr );
   vmcs_read( exit_ctl->msr_load_count );
   vmcs_read( exit_ctl->msr_load_addr );

   /* entry controls */
   vmcs_read( entry_ctl->entry );
   vmcs_read( entry_ctl->msr_load_count );
   vmcs_read( entry_ctl->msr_load_addr );
   vmcs_read( entry_ctl->int_info );
   vmcs_read( entry_ctl->err_code );
   vmcs_read( entry_ctl->insn_len );

   /* host state */
   vmcs_read( h_state->cr0 );
   vmcs_read( h_state->cr3 );
   vmcs_read( h_state->cr4 );
   vmcs_read( h_state->rsp );
   vmcs_read( h_state->rip );
   vmcs_read( h_state->cs );
   vmcs_read( h_state->ss );
   vmcs_read( h_state->ds );
   vmcs_read( h_state->es );
   vmcs_read( h_state->fs );
   vmcs_read( h_state->gs );
   vmcs_read( h_state->tr );
   vmcs_read( h_state->fs_base_addr );
   vmcs_read( h_state->gs_base_addr );
   vmcs_read( h_state->tr_base_addr );
   vmcs_read( h_state->gdtr_base_addr );
   vmcs_read( h_state->idtr_base_addr );
   vmcs_read( h_state->ia32_sysenter_cs_msr );
   vmcs_read( h_state->ia32_sysenter_esp_msr );
   vmcs_read( h_state->ia32_sysenter_eip_msr );

   /* guest state */
   vmcs_read( g_state->cr0 );
   vmcs_read( g_state->cr3 );
   vmcs_read( g_state->cr4 );
   vmcs_read( g_state->dr7 );
   vmcs_read( g_state->rsp );
   vmcs_read( g_state->rip );
   vmcs_read( g_state->rflags );
   vmcs_read( g_state->cs.selector );
   vmcs_read( g_state->ss.selector );
   vmcs_read( g_state->ds.selector );
   vmcs_read( g_state->es.selector );
   vmcs_read( g_state->fs.selector );
   vmcs_read( g_state->gs.selector );
   vmcs_read( g_state->tr.selector );
   vmcs_read( g_state->ldtr.selector );
   vmcs_read( g_state->cs.base_addr );
   vmcs_read( g_state->ss.base_addr );
   vmcs_read( g_state->ds.base_addr );
   vmcs_read( g_state->es.base_addr );
   vmcs_read( g_state->fs.base_addr );
   vmcs_read( g_state->gs.base_addr );
   vmcs_read( g_state->tr.base_addr );
   vmcs_read( g_state->ldtr.base_addr );
   vmcs_read( g_state->cs.limit );
   vmcs_read( g_state->ss.limit );
   vmcs_read( g_state->ds.limit );
   vmcs_read( g_state->es.limit );
   vmcs_read( g_state->fs.limit );
   vmcs_read( g_state->gs.limit );
   vmcs_read( g_state->tr.limit );
   vmcs_read( g_state->ldtr.limit );
   vmcs_read( g_state->cs.attributes );
   vmcs_read( g_state->ss.attributes );
   vmcs_read( g_state->ds.attributes );
   vmcs_read( g_state->es.attributes );
   vmcs_read( g_state->fs.attributes );
   vmcs_read( g_state->gs.attributes );
   vmcs_read( g_state->tr.attributes );
   vmcs_read( g_state->ldtr.attributes );
   vmcs_read( g_state->gdtr.base_addr );
   vmcs_read( g_state->gdtr.limit );
   vmcs_read( g_state->idtr.base_addr );
   vmcs_read( g_state->idtr.limit );
   vmcs_read( g_state->ia32_debugctl_msr );
   vmcs_read( g_state->ia32_sysenter_cs_msr );
   vmcs_read( g_state->ia32_sysenter_esp_msr );
   vmcs_read( g_state->ia32_sysenter_eip_msr );
   vmcs_read( g_state->smbase );
   vmcs_read( g_state->activity_state );
   vmcs_read( g_state->int_state );
   vmcs_read( g_state->dbg_excp );
   vmcs_read( g_state->vmcs_link_ptr );

   /* exit info */
   vmcs_read( exit_info->reason );
   vmcs_read( exit_info->qualification );
   vmcs_read( exit_info->int_info );
   vmcs_read( exit_info->int_err_code );
   vmcs_read( exit_info->idt_info );
   vmcs_read( exit_info->idt_err_code );
   vmcs_read( exit_info->vmexit_insn_len );
   vmcs_read( exit_info->guest_addr );
   vmcs_read( exit_info->vmx_insn_info );
   vmcs_read( exit_info->io_rcx );
   vmcs_read( exit_info->io_rsi );
   vmcs_read( exit_info->io_rdi );
   vmcs_read( exit_info->io_rip );
   vmcs_read( exit_info->vm_insn_err );
}
