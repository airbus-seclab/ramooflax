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

extern info_data_t *info;

void vmx_vmcs_encode()
{
   /* 16-bit fields */
   vmcs_encode(vm_exec_ctrls.vpid, VMCS_FIELD_ENC_EXEC_CTRL_VPID);

   vmcs_encode(vm_state.es.selector, VMCS_FIELD_ENC_GUEST_STATE_ES_SEL);
   vmcs_encode(vm_state.cs.selector, VMCS_FIELD_ENC_GUEST_STATE_CS_SEL);
   vmcs_encode(vm_state.ss.selector, VMCS_FIELD_ENC_GUEST_STATE_SS_SEL);
   vmcs_encode(vm_state.ds.selector, VMCS_FIELD_ENC_GUEST_STATE_DS_SEL);
   vmcs_encode(vm_state.fs.selector, VMCS_FIELD_ENC_GUEST_STATE_FS_SEL);
   vmcs_encode(vm_state.gs.selector, VMCS_FIELD_ENC_GUEST_STATE_GS_SEL);
   vmcs_encode(vm_state.ldtr.selector, VMCS_FIELD_ENC_GUEST_STATE_LDTR_SEL);
   vmcs_encode(vm_state.tr.selector, VMCS_FIELD_ENC_GUEST_STATE_TR_SEL);

   vmcs_encode(vm_host_state.es, VMCS_FIELD_ENC_HOST_STATE_ES_SEL);
   vmcs_encode(vm_host_state.cs, VMCS_FIELD_ENC_HOST_STATE_CS_SEL);
   vmcs_encode(vm_host_state.ss, VMCS_FIELD_ENC_HOST_STATE_SS_SEL);
   vmcs_encode(vm_host_state.ds, VMCS_FIELD_ENC_HOST_STATE_DS_SEL);
   vmcs_encode(vm_host_state.fs, VMCS_FIELD_ENC_HOST_STATE_FS_SEL);
   vmcs_encode(vm_host_state.gs, VMCS_FIELD_ENC_HOST_STATE_GS_SEL);
   vmcs_encode(vm_host_state.tr, VMCS_FIELD_ENC_HOST_STATE_TR_SEL);

   /* 64-bit fields */
   vmcs_encode(vm_exec_ctrls.io_bitmap_a, VMCS_FIELD_ENC_EXEC_CTRL_ADDR_IO_MAP_A);
   vmcs_encode(vm_exec_ctrls.io_bitmap_b, VMCS_FIELD_ENC_EXEC_CTRL_ADDR_IO_MAP_B);
   vmcs_encode(vm_exec_ctrls.msr_bitmap, VMCS_FIELD_ENC_EXEC_CTRL_ADDR_MSR_MAP);
   vmcs_encode(vm_exit_ctrls.msr_store_addr, VMCS_FIELD_ENC_EXIT_CTRL_MSR_STORE_ADDR);
   vmcs_encode(vm_exit_ctrls.msr_load_addr, VMCS_FIELD_ENC_EXIT_CTRL_MSR_LOAD_ADDR);
   vmcs_encode(vm_entry_ctrls.msr_load_addr, VMCS_FIELD_ENC_ENTRY_CTRL_MSR_LOAD_ADDR);
#ifdef CONFIG_VMX_FEAT_VMCS_EXEC_PTR
   vmcs_encode(vm_exec_ctrls.executive_vmcs_ptr, VMCS_FIELD_ENC_EXEC_CTRL_VMCS_PTR);
#endif
   vmcs_encode(vm_exec_ctrls.tsc_offset, VMCS_FIELD_ENC_EXEC_CTRL_TSC_OFFSET);
   vmcs_encode(vm_exec_ctrls.vapic_addr, VMCS_FIELD_ENC_EXEC_CTRL_VAPIC_PAGE_ADDR);
   vmcs_encode(vm_exec_ctrls.apic_addr, VMCS_FIELD_ENC_EXEC_CTRL_APIC_PAGE_ADDR);
   vmcs_encode(vm_exec_ctrls.eptp, VMCS_FIELD_ENC_EXEC_CTRL_EPTP);

   vmcs_encode(vm_exit_info.guest_physical, VMCS_FIELD_ENC_EXIT_INFO_GUEST_PHYSICAL_ADDR);

   vmcs_encode(vm_state.vmcs_link_ptr, VMCS_FIELD_ENC_GUEST_STATE_VMCS_LINK_PTR);
   vmcs_encode(vm_state.ia32_dbgctl, VMCS_FIELD_ENC_GUEST_STATE_IA32_DBG_CTL);
   vmcs_encode(vm_state.ia32_pat, VMCS_FIELD_ENC_GUEST_STATE_IA32_PAT);
   vmcs_encode(vm_state.ia32_efer, VMCS_FIELD_ENC_GUEST_STATE_IA32_EFER);
   vmcs_encode(vm_state.ia32_perf, VMCS_FIELD_ENC_GUEST_STATE_IA32_PERF_GLOBAL);
   vmcs_encode(vm_state.pdpe_0, VMCS_FIELD_ENC_GUEST_STATE_PDPTE0);
   vmcs_encode(vm_state.pdpe_1, VMCS_FIELD_ENC_GUEST_STATE_PDPTE1);
   vmcs_encode(vm_state.pdpe_2, VMCS_FIELD_ENC_GUEST_STATE_PDPTE2);
   vmcs_encode(vm_state.pdpe_3, VMCS_FIELD_ENC_GUEST_STATE_PDPTE3);

   vmcs_encode(vm_host_state.ia32_pat, VMCS_FIELD_ENC_HOST_STATE_IA32_PAT);
   vmcs_encode(vm_host_state.ia32_efer, VMCS_FIELD_ENC_HOST_STATE_IA32_EFER);
   vmcs_encode(vm_host_state.ia32_perf, VMCS_FIELD_ENC_HOST_STATE_IA32_PERF_GLOBAL);

   /* 32-bit fields */
   vmcs_encode(vm_exec_ctrls.pin, VMCS_FIELD_ENC_EXEC_CTRL_PINBASED);
   vmcs_encode(vm_exec_ctrls.proc, VMCS_FIELD_ENC_EXEC_CTRL_PROCBASED);
   vmcs_encode(vm_exec_ctrls.excp_bitmap, VMCS_FIELD_ENC_EXEC_CTRL_EXCP_BITMAP);
   vmcs_encode(vm_exec_ctrls.pagefault_err_code_mask, VMCS_FIELD_ENC_EXEC_CTRL_PGF_ERR_CODE_MASK);
   vmcs_encode(vm_exec_ctrls.pagefault_err_code_match, VMCS_FIELD_ENC_EXEC_CTRL_PGF_ERR_CODE_MATCH);
   vmcs_encode(vm_exec_ctrls.cr3_target_count, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_COUNT);
   vmcs_encode(vm_exit_ctrls.exit, VMCS_FIELD_ENC_EXIT_CTRLS);
   vmcs_encode(vm_exit_ctrls.msr_store_count, VMCS_FIELD_ENC_EXIT_CTRL_MSR_STORE_COUNT);
   vmcs_encode(vm_exit_ctrls.msr_load_count, VMCS_FIELD_ENC_EXIT_CTRL_MSR_LOAD_COUNT);
   vmcs_encode(vm_entry_ctrls.entry, VMCS_FIELD_ENC_ENTRY_CTRLS);
   vmcs_encode(vm_entry_ctrls.msr_load_count, VMCS_FIELD_ENC_ENTRY_CTRL_MSR_LOAD_COUNT);
   vmcs_encode(vm_entry_ctrls.int_info, VMCS_FIELD_ENC_ENTRY_CTRL_INT_INFO);
   vmcs_encode(vm_entry_ctrls.err_code, VMCS_FIELD_ENC_ENTRY_CTRL_EXCP_ERR_CODE);
   vmcs_encode(vm_entry_ctrls.insn_len, VMCS_FIELD_ENC_ENTRY_CTRL_INSN_LEN);
   vmcs_encode(vm_exec_ctrls.tpr_threshold, VMCS_FIELD_ENC_EXEC_CTRL_TPR_THRESHOLD);
   vmcs_encode(vm_exec_ctrls.proc2, VMCS_FIELD_ENC_EXEC_CTRL_PROCBASED_2);
   /* vmcs_encode(vm_exec_ctrls.ple_gap, VMCS_FIELD_ENC_EXEC_CTRL_PLE_GAP); */
   /* vmcs_encode(vm_exec_ctrls.ple_win, VMCS_FIELD_ENC_EXEC_CTRL_PLE_WIN); */

   vmcs_encode(vm_exit_info.vmx_insn_err, VMCS_FIELD_ENC_EXIT_INFO_VM_INSN_ERROR);
   vmcs_encode(vm_exit_info.reason, VMCS_FIELD_ENC_EXIT_INFO_REASON);
   vmcs_encode(vm_exit_info.int_info, VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INT_INFO);
   vmcs_encode(vm_exit_info.int_err_code, VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INT_ERR);
   vmcs_encode(vm_exit_info.idt_info, VMCS_FIELD_ENC_EXIT_INFO_IDT_VECT_INFO);
   vmcs_encode(vm_exit_info.idt_err_code, VMCS_FIELD_ENC_EXIT_INFO_IDT_VECT_ERR);
   vmcs_encode(vm_exit_info.insn_len, VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INSN_LEN);
   vmcs_encode(vm_exit_info.insn_info, VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INSN_INFO);

   vmcs_encode(vm_state.es.limit, VMCS_FIELD_ENC_GUEST_STATE_ES_LIMIT);
   vmcs_encode(vm_state.cs.limit, VMCS_FIELD_ENC_GUEST_STATE_CS_LIMIT);
   vmcs_encode(vm_state.ss.limit, VMCS_FIELD_ENC_GUEST_STATE_SS_LIMIT);
   vmcs_encode(vm_state.ds.limit, VMCS_FIELD_ENC_GUEST_STATE_DS_LIMIT);
   vmcs_encode(vm_state.fs.limit, VMCS_FIELD_ENC_GUEST_STATE_FS_LIMIT);
   vmcs_encode(vm_state.gs.limit, VMCS_FIELD_ENC_GUEST_STATE_GS_LIMIT);
   vmcs_encode(vm_state.ldtr.limit, VMCS_FIELD_ENC_GUEST_STATE_LDTR_LIMIT);
   vmcs_encode(vm_state.tr.limit, VMCS_FIELD_ENC_GUEST_STATE_TR_LIMIT);
   vmcs_encode(vm_state.gdtr.limit, VMCS_FIELD_ENC_GUEST_STATE_GDTR_LIMIT);
   vmcs_encode(vm_state.idtr.limit, VMCS_FIELD_ENC_GUEST_STATE_IDTR_LIMIT);
   vmcs_encode(vm_state.es.attributes, VMCS_FIELD_ENC_GUEST_STATE_ES_ACCESS_RIGHTS);
   vmcs_encode(vm_state.cs.attributes, VMCS_FIELD_ENC_GUEST_STATE_CS_ACCESS_RIGHTS);
   vmcs_encode(vm_state.ss.attributes, VMCS_FIELD_ENC_GUEST_STATE_SS_ACCESS_RIGHTS);
   vmcs_encode(vm_state.ds.attributes, VMCS_FIELD_ENC_GUEST_STATE_DS_ACCESS_RIGHTS);
   vmcs_encode(vm_state.fs.attributes, VMCS_FIELD_ENC_GUEST_STATE_FS_ACCESS_RIGHTS);
   vmcs_encode(vm_state.gs.attributes, VMCS_FIELD_ENC_GUEST_STATE_GS_ACCESS_RIGHTS);
   vmcs_encode(vm_state.ldtr.attributes, VMCS_FIELD_ENC_GUEST_STATE_LDTR_ACCESS_RIGHTS);
   vmcs_encode(vm_state.tr.attributes, VMCS_FIELD_ENC_GUEST_STATE_TR_ACCESS_RIGHTS);
   vmcs_encode(vm_state.interrupt, VMCS_FIELD_ENC_GUEST_STATE_INT_STATE);
   vmcs_encode(vm_state.activity, VMCS_FIELD_ENC_GUEST_STATE_ACTIVITY_STATE);
#ifdef CONFIG_VMX_FEAT_VMCS_SMBASE
   vmcs_encode(vm_state.smbase, VMCS_FIELD_ENC_GUEST_STATE_SMBASE);
#endif
   vmcs_encode(vm_state.ia32_sysenter_cs, VMCS_FIELD_ENC_GUEST_STATE_IA32_SYSENTER_CS);
   vmcs_encode(vm_state.preempt_timer, VMCS_FIELD_ENC_GUEST_STATE_PREEMPT_TIMER);

   vmcs_encode(vm_host_state.ia32_sysenter_cs, VMCS_FIELD_ENC_HOST_STATE_IA32_SYSENTER_CS);

   /* Natural fields */
   vmcs_encode(vm_exec_ctrls.cr0_mask, VMCS_FIELD_ENC_EXEC_CTRL_CR0_GUEST_HOST_MASK);
   vmcs_encode(vm_exec_ctrls.cr4_mask, VMCS_FIELD_ENC_EXEC_CTRL_CR4_GUEST_HOST_MASK);
   vmcs_encode(vm_exec_ctrls.cr0_read_shadow, VMCS_FIELD_ENC_EXEC_CTRL_CR0_READ_SHADOW);
   vmcs_encode(vm_exec_ctrls.cr4_read_shadow, VMCS_FIELD_ENC_EXEC_CTRL_CR4_READ_SHADOW);
   vmcs_encode(vm_exec_ctrls.cr3_target_0, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_0);
   vmcs_encode(vm_exec_ctrls.cr3_target_1, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_1);
   vmcs_encode(vm_exec_ctrls.cr3_target_2, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_2);
   vmcs_encode(vm_exec_ctrls.cr3_target_3, VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_3);

   vmcs_encode(vm_exit_info.qualification, VMCS_FIELD_ENC_EXIT_INFO_QUALIFICATION);
   vmcs_encode(vm_exit_info.io_rcx, VMCS_FIELD_ENC_EXIT_INFO_IO_RCX);
   vmcs_encode(vm_exit_info.io_rsi, VMCS_FIELD_ENC_EXIT_INFO_IO_RSI);
   vmcs_encode(vm_exit_info.io_rdi, VMCS_FIELD_ENC_EXIT_INFO_IO_RDI);
   vmcs_encode(vm_exit_info.io_rip, VMCS_FIELD_ENC_EXIT_INFO_IO_RIP);
   vmcs_encode(vm_exit_info.guest_linear, VMCS_FIELD_ENC_EXIT_INFO_GUEST_LINEAR_ADDR);

   vmcs_encode(vm_state.cr0, VMCS_FIELD_ENC_GUEST_STATE_CR0);
   vmcs_encode(vm_state.cr3, VMCS_FIELD_ENC_GUEST_STATE_CR3);
   vmcs_encode(vm_state.cr4, VMCS_FIELD_ENC_GUEST_STATE_CR4);
   vmcs_encode(vm_state.es.base, VMCS_FIELD_ENC_GUEST_STATE_ES_BASE);
   vmcs_encode(vm_state.cs.base, VMCS_FIELD_ENC_GUEST_STATE_CS_BASE);
   vmcs_encode(vm_state.ss.base, VMCS_FIELD_ENC_GUEST_STATE_SS_BASE);
   vmcs_encode(vm_state.ds.base, VMCS_FIELD_ENC_GUEST_STATE_DS_BASE);
   vmcs_encode(vm_state.fs.base, VMCS_FIELD_ENC_GUEST_STATE_FS_BASE);
   vmcs_encode(vm_state.gs.base, VMCS_FIELD_ENC_GUEST_STATE_GS_BASE);
   vmcs_encode(vm_state.tr.base, VMCS_FIELD_ENC_GUEST_STATE_TR_BASE);
   vmcs_encode(vm_state.ldtr.base, VMCS_FIELD_ENC_GUEST_STATE_LDTR_BASE);
   vmcs_encode(vm_state.gdtr.base, VMCS_FIELD_ENC_GUEST_STATE_GDTR_BASE);
   vmcs_encode(vm_state.idtr.base, VMCS_FIELD_ENC_GUEST_STATE_IDTR_BASE);
   vmcs_encode(vm_state.dr7, VMCS_FIELD_ENC_GUEST_STATE_DR7);
   vmcs_encode(vm_state.rsp, VMCS_FIELD_ENC_GUEST_STATE_RSP);
   vmcs_encode(vm_state.rip, VMCS_FIELD_ENC_GUEST_STATE_RIP);
   vmcs_encode(vm_state.rflags, VMCS_FIELD_ENC_GUEST_STATE_RFLAGS);
   vmcs_encode(vm_state.dbg_excp, VMCS_FIELD_ENC_GUEST_STATE_PENDING_DBG_EXCP);
   vmcs_encode(vm_state.ia32_sysenter_esp, VMCS_FIELD_ENC_GUEST_STATE_IA32_SYSENTER_ESP);
   vmcs_encode(vm_state.ia32_sysenter_eip, VMCS_FIELD_ENC_GUEST_STATE_IA32_SYSENTER_EIP);

   vmcs_encode(vm_host_state.cr0, VMCS_FIELD_ENC_HOST_STATE_CR0);
   vmcs_encode(vm_host_state.cr3, VMCS_FIELD_ENC_HOST_STATE_CR3);
   vmcs_encode(vm_host_state.cr4, VMCS_FIELD_ENC_HOST_STATE_CR4);
   vmcs_encode(vm_host_state.fs_base, VMCS_FIELD_ENC_HOST_STATE_FS_BASE);
   vmcs_encode(vm_host_state.gs_base, VMCS_FIELD_ENC_HOST_STATE_GS_BASE);
   vmcs_encode(vm_host_state.tr_base, VMCS_FIELD_ENC_HOST_STATE_TR_BASE);
   vmcs_encode(vm_host_state.gdtr_base, VMCS_FIELD_ENC_HOST_STATE_GDTR_BASE);
   vmcs_encode(vm_host_state.idtr_base, VMCS_FIELD_ENC_HOST_STATE_IDTR_BASE);
   vmcs_encode(vm_host_state.ia32_sysenter_esp, VMCS_FIELD_ENC_HOST_STATE_IA32_SYSENTER_ESP);
   vmcs_encode(vm_host_state.ia32_sysenter_eip, VMCS_FIELD_ENC_HOST_STATE_IA32_SYSENTER_EIP);
   vmcs_encode(vm_host_state.rsp, VMCS_FIELD_ENC_HOST_STATE_RSP);
   vmcs_encode(vm_host_state.rip, VMCS_FIELD_ENC_HOST_STATE_RIP);

   /* Fake fields */
   vmcs_fake_it(vm_state.cr2);
   vmcs_fake_it(vm_state.dr6);
}
