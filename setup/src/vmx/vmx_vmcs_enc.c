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
#include <info_data.h>

void vmx_vmcs_encode(info_data_t *info)
{
   vmcs_exec_ctl_t   *exec_ctl  = exec_ctrls(info);
   vmcs_exit_ctl_t   *exit_ctl  = exit_ctrls(info);
   vmcs_entry_ctl_t  *entry_ctl = entry_ctrls(info);
   vmcs_host_t       *h_state   = host_state(info);
   vmcs_guest_t      *g_state   = guest_state(info);
   vmcs_exit_info_t  *exit_info = exit_info(info);

   /* execution controls */
   vmcs_encode( exec_ctl->pin, VMCS_FIELD_ENC_EXEC_CTRL_PINBASED );
   vmcs_encode( exec_ctl->proc, VMCS_FIELD_ENC_EXEC_CTRL_PROCBASED );
   vmcs_encode( exec_ctl->excp_bitmap, VMCS_FIELD_ENC_EXEC_CTRL_EXCP_BITMAP );
   vmcs_encode( exec_ctl->pagefault_err_code_mask, VMCS_FIELD_ENC_EXEC_CTRL_PAGEFAULT_ERR_CODE_MASK );
   vmcs_encode( exec_ctl->pagefault_err_code_match, VMCS_FIELD_ENC_EXEC_CTRL_PAGEFAULT_ERR_CODE_MATCH );
   vmcs_encode( exec_ctl->io_bitmap_a, VMCS_FIELD_ENC_EXEC_CTRL_ADDR_IO_MAP_A_FULL );
   vmcs_encode( exec_ctl->io_bitmap_b, VMCS_FIELD_ENC_EXEC_CTRL_ADDR_IO_MAP_B_FULL );
   vmcs_encode( exec_ctl->tsc_offset, VMCS_FIELD_ENC_EXEC_CTRL_TSC_OFFSET_FULL );
   vmcs_encode( exec_ctl->cr0_mask, VMCS_FIELD_ENC_EXEC_CTRL_CR0_GUEST_HOST_MASK );
   vmcs_encode( exec_ctl->cr4_mask, VMCS_FIELD_ENC_EXEC_CTRL_CR4_GUEST_HOST_MASK );
   vmcs_encode( exec_ctl->cr0_read_shadow, VMCS_FIELD_ENC_EXEC_CTRL_CR0_READ_SHADOW );
   vmcs_encode( exec_ctl->cr4_read_shadow, VMCS_FIELD_ENC_EXEC_CTRL_CR4_READ_SHADOW );
   vmcs_encode( exec_ctl->cr3_target_0, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_0 );
   vmcs_encode( exec_ctl->cr3_target_1, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_1 );
   vmcs_encode( exec_ctl->cr3_target_2, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_2 );
   vmcs_encode( exec_ctl->cr3_target_3, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_3 );
   vmcs_encode( exec_ctl->cr3_target_count, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_COUNT );
   vmcs_encode( exec_ctl->v_apic_page, VMCS_FIELD_ENC_EXEC_CTRL_VAPIC_PAGE_ADDR_FULL );
   vmcs_encode( exec_ctl->tpr_threshold, VMCS_FIELD_ENC_EXEC_CTRL_TPR_THRESHOLD );
   vmcs_encode( exec_ctl->msr_bitmap, VMCS_FIELD_ENC_EXEC_CTRL_ADDR_MSR_MAP_FULL );
   vmcs_encode( exec_ctl->executive_vmcs_ptr, VMCS_FIELD_ENC_EXEC_CTRL_VMCS_PTR_FULL );

   /* exit controls */
   vmcs_encode( exit_ctl->exit, VMCS_FIELD_ENC_EXIT_CTRLS );
   vmcs_encode( exit_ctl->msr_store_count, VMCS_FIELD_ENC_EXIT_CTRL_MSR_STORE_COUNT );
   vmcs_encode( exit_ctl->msr_store_addr, VMCS_FIELD_ENC_EXIT_CTRL_MSR_STORE_ADDR_FULL );
   vmcs_encode( exit_ctl->msr_load_count, VMCS_FIELD_ENC_EXIT_CTRL_MSR_LOAD_COUNT );
   vmcs_encode( exit_ctl->msr_load_addr, VMCS_FIELD_ENC_EXIT_CTRL_MSR_LOAD_ADDR_FULL );

   /* entry controls */
   vmcs_encode( entry_ctl->entry, VMCS_FIELD_ENC_ENTRY_CTRLS );
   vmcs_encode( entry_ctl->msr_load_count, VMCS_FIELD_ENC_ENTRY_CTRL_MSR_LOAD_COUNT );
   vmcs_encode( entry_ctl->msr_load_addr, VMCS_FIELD_ENC_ENTRY_CTRL_MSR_LOAD_ADDR_FULL );
   vmcs_encode( entry_ctl->int_info, VMCS_FIELD_ENC_ENTRY_CTRL_INT_INFO );
   vmcs_encode( entry_ctl->err_code, VMCS_FIELD_ENC_ENTRY_CTRL_EXCP_ERR_CODE );
   vmcs_encode( entry_ctl->insn_len, VMCS_FIELD_ENC_ENTRY_CTRL_INSN_LEN );

   /* host state */
   vmcs_encode( h_state->cr0, VMCS_FIELD_ENC_HOST_STATE_CR0 );
   vmcs_encode( h_state->cr3, VMCS_FIELD_ENC_HOST_STATE_CR3 );
   vmcs_encode( h_state->cr4, VMCS_FIELD_ENC_HOST_STATE_CR4 );
   vmcs_encode( h_state->rsp, VMCS_FIELD_ENC_HOST_STATE_RSP );
   vmcs_encode( h_state->rip, VMCS_FIELD_ENC_HOST_STATE_RIP );
   vmcs_encode( h_state->cs, VMCS_FIELD_ENC_HOST_STATE_CS_SEL );
   vmcs_encode( h_state->ss, VMCS_FIELD_ENC_HOST_STATE_SS_SEL );
   vmcs_encode( h_state->ds, VMCS_FIELD_ENC_HOST_STATE_DS_SEL );
   vmcs_encode( h_state->es, VMCS_FIELD_ENC_HOST_STATE_ES_SEL );
   vmcs_encode( h_state->fs, VMCS_FIELD_ENC_HOST_STATE_FS_SEL );
   vmcs_encode( h_state->gs, VMCS_FIELD_ENC_HOST_STATE_GS_SEL );
   vmcs_encode( h_state->tr, VMCS_FIELD_ENC_HOST_STATE_TR_SEL );
   vmcs_encode( h_state->fs_base_addr, VMCS_FIELD_ENC_HOST_STATE_FS_BASE );
   vmcs_encode( h_state->gs_base_addr, VMCS_FIELD_ENC_HOST_STATE_GS_BASE );
   vmcs_encode( h_state->tr_base_addr, VMCS_FIELD_ENC_HOST_STATE_TR_BASE );
   vmcs_encode( h_state->gdtr_base_addr, VMCS_FIELD_ENC_HOST_STATE_GDTR_BASE );
   vmcs_encode( h_state->idtr_base_addr, VMCS_FIELD_ENC_HOST_STATE_IDTR_BASE );
   vmcs_encode( h_state->ia32_sysenter_cs_msr, VMCS_FIELD_ENC_HOST_STATE_IA32_SYSENTER_CS );
   vmcs_encode( h_state->ia32_sysenter_esp_msr, VMCS_FIELD_ENC_HOST_STATE_IA32_SYSENTER_ESP );
   vmcs_encode( h_state->ia32_sysenter_eip_msr, VMCS_FIELD_ENC_HOST_STATE_IA32_SYSENTER_EIP );

   /* guest state */
   vmcs_encode( g_state->cr0, VMCS_FIELD_ENC_GUEST_STATE_CR0 );
   vmcs_encode( g_state->cr3, VMCS_FIELD_ENC_GUEST_STATE_CR3 );
   vmcs_encode( g_state->cr4, VMCS_FIELD_ENC_GUEST_STATE_CR4 );
   vmcs_encode( g_state->dr7, VMCS_FIELD_ENC_GUEST_STATE_DR7 );
   vmcs_encode( g_state->rsp, VMCS_FIELD_ENC_GUEST_STATE_RSP );
   vmcs_encode( g_state->rip, VMCS_FIELD_ENC_GUEST_STATE_RIP );
   vmcs_encode( g_state->rflags, VMCS_FIELD_ENC_GUEST_STATE_RFLAGS );
   vmcs_encode( g_state->cs.selector, VMCS_FIELD_ENC_GUEST_STATE_CS_SEL );
   vmcs_encode( g_state->ss.selector, VMCS_FIELD_ENC_GUEST_STATE_SS_SEL );
   vmcs_encode( g_state->ds.selector, VMCS_FIELD_ENC_GUEST_STATE_DS_SEL );
   vmcs_encode( g_state->es.selector, VMCS_FIELD_ENC_GUEST_STATE_ES_SEL );
   vmcs_encode( g_state->fs.selector, VMCS_FIELD_ENC_GUEST_STATE_FS_SEL );
   vmcs_encode( g_state->gs.selector, VMCS_FIELD_ENC_GUEST_STATE_GS_SEL );
   vmcs_encode( g_state->tr.selector, VMCS_FIELD_ENC_GUEST_STATE_TR_SEL );
   vmcs_encode( g_state->ldtr.selector, VMCS_FIELD_ENC_GUEST_STATE_LDTR_SEL );
   vmcs_encode( g_state->cs.base_addr, VMCS_FIELD_ENC_GUEST_STATE_CS_BASE );
   vmcs_encode( g_state->ss.base_addr, VMCS_FIELD_ENC_GUEST_STATE_SS_BASE );
   vmcs_encode( g_state->ds.base_addr, VMCS_FIELD_ENC_GUEST_STATE_DS_BASE );
   vmcs_encode( g_state->es.base_addr, VMCS_FIELD_ENC_GUEST_STATE_ES_BASE );
   vmcs_encode( g_state->fs.base_addr, VMCS_FIELD_ENC_GUEST_STATE_FS_BASE );
   vmcs_encode( g_state->gs.base_addr, VMCS_FIELD_ENC_GUEST_STATE_GS_BASE );
   vmcs_encode( g_state->tr.base_addr, VMCS_FIELD_ENC_GUEST_STATE_TR_BASE );
   vmcs_encode( g_state->ldtr.base_addr, VMCS_FIELD_ENC_GUEST_STATE_LDTR_BASE );
   vmcs_encode( g_state->cs.limit, VMCS_FIELD_ENC_GUEST_STATE_CS_LIMIT );
   vmcs_encode( g_state->ss.limit, VMCS_FIELD_ENC_GUEST_STATE_SS_LIMIT );
   vmcs_encode( g_state->ds.limit, VMCS_FIELD_ENC_GUEST_STATE_DS_LIMIT );
   vmcs_encode( g_state->es.limit, VMCS_FIELD_ENC_GUEST_STATE_ES_LIMIT );
   vmcs_encode( g_state->fs.limit, VMCS_FIELD_ENC_GUEST_STATE_FS_LIMIT );
   vmcs_encode( g_state->gs.limit, VMCS_FIELD_ENC_GUEST_STATE_GS_LIMIT );
   vmcs_encode( g_state->tr.limit, VMCS_FIELD_ENC_GUEST_STATE_TR_LIMIT );
   vmcs_encode( g_state->ldtr.limit, VMCS_FIELD_ENC_GUEST_STATE_LDTR_LIMIT );
   vmcs_encode( g_state->cs.attributes, VMCS_FIELD_ENC_GUEST_STATE_CS_ACCESS_RIGHTS );
   vmcs_encode( g_state->ss.attributes, VMCS_FIELD_ENC_GUEST_STATE_SS_ACCESS_RIGHTS );
   vmcs_encode( g_state->ds.attributes, VMCS_FIELD_ENC_GUEST_STATE_DS_ACCESS_RIGHTS );
   vmcs_encode( g_state->es.attributes, VMCS_FIELD_ENC_GUEST_STATE_ES_ACCESS_RIGHTS );
   vmcs_encode( g_state->fs.attributes, VMCS_FIELD_ENC_GUEST_STATE_FS_ACCESS_RIGHTS );
   vmcs_encode( g_state->gs.attributes, VMCS_FIELD_ENC_GUEST_STATE_GS_ACCESS_RIGHTS );
   vmcs_encode( g_state->tr.attributes, VMCS_FIELD_ENC_GUEST_STATE_TR_ACCESS_RIGHTS );
   vmcs_encode( g_state->ldtr.attributes, VMCS_FIELD_ENC_GUEST_STATE_LDTR_ACCESS_RIGHTS );
   vmcs_encode( g_state->gdtr.base_addr, VMCS_FIELD_ENC_GUEST_STATE_GDTR_BASE );
   vmcs_encode( g_state->gdtr.limit, VMCS_FIELD_ENC_GUEST_STATE_GDTR_LIMIT );
   vmcs_encode( g_state->idtr.base_addr, VMCS_FIELD_ENC_GUEST_STATE_IDTR_BASE );
   vmcs_encode( g_state->idtr.limit, VMCS_FIELD_ENC_GUEST_STATE_IDTR_LIMIT );
   vmcs_encode( g_state->ia32_debugctl_msr, VMCS_FIELD_ENC_GUEST_STATE_IA32_DBG_CTL_FULL );
   vmcs_encode( g_state->ia32_sysenter_cs_msr, VMCS_FIELD_ENC_GUEST_STATE_IA32_SYSENTER_CS );
   vmcs_encode( g_state->ia32_sysenter_esp_msr, VMCS_FIELD_ENC_GUEST_STATE_IA32_SYSENTER_ESP );
   vmcs_encode( g_state->ia32_sysenter_eip_msr, VMCS_FIELD_ENC_GUEST_STATE_IA32_SYSENTER_EIP );
   vmcs_encode( g_state->smbase, VMCS_FIELD_ENC_GUEST_STATE_SMBASE );
   vmcs_encode( g_state->activity_state, VMCS_FIELD_ENC_GUEST_STATE_ACTIVITY_STATE );
   vmcs_encode( g_state->int_state, VMCS_FIELD_ENC_GUEST_STATE_INT_STATE );
   vmcs_encode( g_state->dbg_excp, VMCS_FIELD_ENC_GUEST_STATE_PENDING_DBG_EXCP );
   vmcs_encode( g_state->vmcs_link_ptr, VMCS_FIELD_ENC_GUEST_STATE_VMCS_LINK_PTR_FULL );

   /* exit info */
   vmcs_encode( exit_info->reason, VMCS_FIELD_ENC_EXIT_INFO_REASON );
   vmcs_encode( exit_info->qualification, VMCS_FIELD_ENC_EXIT_INFO_QUALIFICATION );
   vmcs_encode( exit_info->int_info, VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INT_INFO );
   vmcs_encode( exit_info->int_err_code, VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INT_ERR );
   vmcs_encode( exit_info->idt_info, VMCS_FIELD_ENC_EXIT_INFO_IDT_VECT_INFO );
   vmcs_encode( exit_info->idt_err_code, VMCS_FIELD_ENC_EXIT_INFO_IDT_VECT_ERR );
   vmcs_encode( exit_info->vmexit_insn_len, VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INSN_LEN );
   vmcs_encode( exit_info->guest_addr, VMCS_FIELD_ENC_EXIT_INFO_GUEST_LINEAR_ADDR );
   vmcs_encode( exit_info->vmx_insn_info, VMCS_FIELD_ENC_EXIT_INFO_VMX_INSN_INFO );
   vmcs_encode( exit_info->io_rcx, VMCS_FIELD_ENC_EXIT_INFO_IO_RCX );
   vmcs_encode( exit_info->io_rsi, VMCS_FIELD_ENC_EXIT_INFO_IO_RSI );
   vmcs_encode( exit_info->io_rdi, VMCS_FIELD_ENC_EXIT_INFO_IO_RDI );
   vmcs_encode( exit_info->io_rip, VMCS_FIELD_ENC_EXIT_INFO_IO_RIP );
   vmcs_encode( exit_info->vm_insn_err, VMCS_FIELD_ENC_EXIT_INFO_VM_INSN_ERROR );
}
