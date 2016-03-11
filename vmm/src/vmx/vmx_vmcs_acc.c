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
#include <vmx_vmcs_acc.h>
#include <vmx_insn.h>
#include <vmx_vmm.h>
#include <cr.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** VMX insn operates on 64 bits in long mode
** so we ensure allocation
*/
void __regparm__(2) __vmcs_force_read(raw64_t *val, vmcs_field_enc_t enc)
{
   raw64_t tmp;

   if(!enc.fake)
   {
      vmx_insn_err_t vmx_err;

      if(!vmx_vmread(&vmx_err, &tmp.raw, enc.raw))
	 panic("vmread(0x%x) err %d\n", enc.raw, vmx_err.raw);

      switch(enc.fwidth)
      {
      case VMCS_FIELD_ENC_FIELD_WIDTH_16: val->wlow = tmp.wlow; break;
      case VMCS_FIELD_ENC_FIELD_WIDTH_32: val->low  = tmp.low;  break;
      default:                            val->raw  = tmp.raw;  break;
      }

      //debug(VMX,"vmread(0x%x) = 0x%X\n", enc.raw, tmp.raw);
      return;
   }

   if(val == (raw64_t*)&vm_state.cr2)
      vm_state.cr2.raw = get_cr2();
   else if(val == (raw64_t*)&vm_state.dr6)
      vm_state.dr6.raw = get_dr6();
}

void __regparm__(2) __vmcs_force_flush(raw64_t *val, vmcs_field_enc_t enc)
{
   if(!enc.fake)
   {
      vmx_insn_err_t vmx_err;

      if(!vmx_vmwrite(&vmx_err, val->raw, enc.raw))
	 panic("vmwrite(0x%x, 0x%x) err %d\n", val->raw, enc.raw, vmx_err.raw);

      //debug(VMX,"vmwrite(0x%x) = 0x%X\n", enc.raw, val->raw);
      return;
   }

   if(val == (raw64_t*)&vm_state.cr2)
      set_cr2(vm_state.cr2.raw);
   else if(val == (raw64_t*)&vm_state.dr6)
      set_dr6(vm_state.dr6.raw);
}

void vmx_vmcs_commit()
{
   /* 16-bit fields */
   vmcs_flush(vm_exec_ctrls.vpid);

   vmcs_flush(vm_state.es.selector);
   vmcs_flush(vm_state.cs.selector);
   vmcs_flush(vm_state.ss.selector);
   vmcs_flush(vm_state.ds.selector);
   vmcs_flush(vm_state.fs.selector);
   vmcs_flush(vm_state.gs.selector);
   vmcs_flush(vm_state.ldtr.selector);
   vmcs_flush(vm_state.tr.selector);

   /* vmcs_flush(vm_host_state.es); */
   /* vmcs_flush(vm_host_state.cs); */
   /* vmcs_flush(vm_host_state.ss); */
   /* vmcs_flush(vm_host_state.ds); */
   /* vmcs_flush(vm_host_state.fs); */
   /* vmcs_flush(vm_host_state.gs); */
   /* vmcs_flush(vm_host_state.tr); */

   /* 64-bit fields */
   vmcs_flush(vm_exec_ctrls.io_bitmap_a);
   vmcs_flush(vm_exec_ctrls.io_bitmap_b);
   vmcs_flush(vm_exec_ctrls.msr_bitmap);
   vmcs_flush(vm_exit_ctrls.msr_store_addr);
   vmcs_flush(vm_exit_ctrls.msr_load_addr);
   vmcs_flush(vm_entry_ctrls.msr_load_addr);
#ifdef CONFIG_VMX_FEAT_VMCS_EXEC_PTR
   vmcs_flush(vm_exec_ctrls.executive_vmcs_ptr);
#endif
   vmcs_flush(vm_exec_ctrls.tsc_offset);

   if(info->vm.vmx_fx_proc.allow_1.tprs)
      vmcs_flush(vm_exec_ctrls.vapic_addr);

   if(info->vm.vmx_fx_proc2.allow_1.vapic)
      vmcs_flush(vm_exec_ctrls.apic_addr);

   vmcs_flush(vm_exec_ctrls.eptp);
   vmcs_flush(vm_exit_info.guest_physical);
   vmcs_flush(vm_state.vmcs_link_ptr);
   vmcs_flush(vm_state.ia32_dbgctl);

   vmcs_flush_fixed(vm_entry_ctrls.entry, info->vm.vmx_fx_entry);

   if(vm_entry_ctrls.entry.load_ia32_pat)
      vmcs_flush(vm_state.ia32_pat);

   if(vm_entry_ctrls.entry.load_ia32_efer)
      vmcs_flush(vm_state.ia32_efer);

   if(vm_entry_ctrls.entry.load_ia32_perf)
      vmcs_flush(vm_state.ia32_perf);

   vmcs_flush(vm_state.pdpe_0);
   vmcs_flush(vm_state.pdpe_1);
   vmcs_flush(vm_state.pdpe_2);
   vmcs_flush(vm_state.pdpe_3);

   /* vmcs_flush(vm_host_state.ia32_pat); */
   /* vmcs_flush(vm_host_state.ia32_efer); */
   /* vmcs_flush(vm_host_state.ia32_perf); */

   /* 32-bit fields */
   vmcs_flush_fixed(vm_exec_ctrls.pin, info->vm.vmx_fx_pin);
   vmcs_flush_fixed(vm_exec_ctrls.proc, info->vm.vmx_fx_proc);
   vmcs_flush(vm_exec_ctrls.excp_bitmap);
   vmcs_flush(vm_exec_ctrls.pagefault_err_code_mask);
   vmcs_flush(vm_exec_ctrls.pagefault_err_code_match);
   vmcs_flush(vm_exec_ctrls.cr3_target_count);
   vmcs_flush_fixed(vm_exit_ctrls.exit, info->vm.vmx_fx_exit);
   vmcs_flush(vm_exit_ctrls.msr_store_count);
   vmcs_flush(vm_exit_ctrls.msr_load_count);

   vmcs_flush(vm_entry_ctrls.msr_load_count);
   vmcs_flush(vm_entry_ctrls.int_info);
   vmcs_flush(vm_entry_ctrls.err_code);
   vmcs_flush(vm_entry_ctrls.insn_len);
   vmcs_flush(vm_exec_ctrls.tpr_threshold);
   vmcs_flush_fixed(vm_exec_ctrls.proc2, info->vm.vmx_fx_proc2);
   /* vmcs_flush(vm_exec_ctrls.ple_gap); */
   /* vmcs_flush(vm_exec_ctrls.ple_win); */

   vmcs_flush(vm_exit_info.vmx_insn_err);
   vmcs_flush(vm_exit_info.reason);
   vmcs_flush(vm_exit_info.int_info);
   vmcs_flush(vm_exit_info.int_err_code);
   vmcs_flush(vm_exit_info.idt_info);
   vmcs_flush(vm_exit_info.idt_err_code);
   vmcs_flush(vm_exit_info.insn_len);
   vmcs_flush(vm_exit_info.insn_info);

   vmcs_flush(vm_state.es.limit);
   vmcs_flush(vm_state.cs.limit);
   vmcs_flush(vm_state.ss.limit);
   vmcs_flush(vm_state.ds.limit);
   vmcs_flush(vm_state.fs.limit);
   vmcs_flush(vm_state.gs.limit);
   vmcs_flush(vm_state.ldtr.limit);
   vmcs_flush(vm_state.tr.limit);
   vmcs_flush(vm_state.gdtr.limit);
   vmcs_flush(vm_state.idtr.limit);
   vmcs_flush(vm_state.es.attributes);
   vmcs_flush(vm_state.cs.attributes);
   vmcs_flush(vm_state.ss.attributes);
   vmcs_flush(vm_state.ds.attributes);
   vmcs_flush(vm_state.fs.attributes);
   vmcs_flush(vm_state.gs.attributes);
   vmcs_flush(vm_state.ldtr.attributes);
   vmcs_flush(vm_state.tr.attributes);
   vmcs_flush(vm_state.interrupt);
   vmcs_flush(vm_state.activity);
#ifdef CONFIG_VMX_FEAT_VMCS_SMBASE
   vmcs_flush(vm_state.smbase);
#endif
   vmcs_flush(vm_state.ia32_sysenter_cs);

   if(info->vm.vmx_fx_pin.allow_1.preempt)
      vmcs_flush(vm_state.preempt_timer);

   /* vmcs_flush(vm_host_state.ia32_sysenter_cs); */

   /* Natural fields */
   vmcs_flush(vm_exec_ctrls.cr0_mask);
   vmcs_flush(vm_exec_ctrls.cr4_mask);
   vmcs_flush(vm_exec_ctrls.cr0_read_shadow);
   vmcs_flush(vm_exec_ctrls.cr4_read_shadow);
   vmcs_flush(vm_exec_ctrls.cr3_target_0);
   vmcs_flush(vm_exec_ctrls.cr3_target_1);
   vmcs_flush(vm_exec_ctrls.cr3_target_2);
   vmcs_flush(vm_exec_ctrls.cr3_target_3);

   vmcs_flush(vm_exit_info.qualification);
   vmcs_flush(vm_exit_info.io_rcx);
   vmcs_flush(vm_exit_info.io_rsi);
   vmcs_flush(vm_exit_info.io_rdi);
   vmcs_flush(vm_exit_info.io_rip);
   vmcs_flush(vm_exit_info.guest_linear);

   vmcs_flush(vm_state.cr3);

   vmcs_flush_fixed(vm_state.cr0, info->vm.vmx_fx_cr0);
   vmcs_flush_fixed(vm_state.cr4, info->vm.vmx_fx_cr4);

   vmcs_flush(vm_state.es.base);
   vmcs_flush(vm_state.cs.base);
   vmcs_flush(vm_state.ss.base);
   vmcs_flush(vm_state.ds.base);
   vmcs_flush(vm_state.fs.base);
   vmcs_flush(vm_state.gs.base);
   vmcs_flush(vm_state.tr.base);
   vmcs_flush(vm_state.ldtr.base);
   vmcs_flush(vm_state.gdtr.base);
   vmcs_flush(vm_state.idtr.base);
   vmcs_flush(vm_state.dr7);
   vmcs_flush(vm_state.rsp);
   vmcs_flush(vm_state.rip);
   vmcs_flush(vm_state.rflags);
   vmcs_flush(vm_state.dbg_excp);
   vmcs_flush(vm_state.ia32_sysenter_esp);
   vmcs_flush(vm_state.ia32_sysenter_eip);

   /* vmcs_flush(vm_host_state.cr0); */
   /* vmcs_flush(vm_host_state.cr3); */
   /* vmcs_flush(vm_host_state.cr4); */
   /* vmcs_flush(vm_host_state.fs_base); */
   /* vmcs_flush(vm_host_state.gs_base); */
   /* vmcs_flush(vm_host_state.tr_base); */
   /* vmcs_flush(vm_host_state.gdtr_base); */
   /* vmcs_flush(vm_host_state.idtr_base); */
   /* vmcs_flush(vm_host_state.ia32_sysenter_esp); */
   /* vmcs_flush(vm_host_state.ia32_sysenter_eip); */
   /* vmcs_flush(vm_host_state.rsp); */
   /* vmcs_flush(vm_host_state.rip); */

   /* Fake fields */
   vmcs_flush(vm_state.cr2);
   vmcs_flush(vm_state.dr6);
}

void vmx_vmcs_collect()
{
   /* 16-bit fields */
   vmcs_read(vm_exec_ctrls.vpid);

   vmcs_read(vm_state.es.selector);
   vmcs_read(vm_state.cs.selector);
   vmcs_read(vm_state.ss.selector);
   vmcs_read(vm_state.ds.selector);
   vmcs_read(vm_state.fs.selector);
   vmcs_read(vm_state.gs.selector);
   vmcs_read(vm_state.ldtr.selector);
   vmcs_read(vm_state.tr.selector);

   /* vmcs_read(vm_host_state.es); */
   /* vmcs_read(vm_host_state.cs); */
   /* vmcs_read(vm_host_state.ss); */
   /* vmcs_read(vm_host_state.ds); */
   /* vmcs_read(vm_host_state.fs); */
   /* vmcs_read(vm_host_state.gs); */
   /* vmcs_read(vm_host_state.tr); */

   /* 64-bit fields */
   vmcs_read(vm_exec_ctrls.io_bitmap_a);
   vmcs_read(vm_exec_ctrls.io_bitmap_b);
   vmcs_read(vm_exec_ctrls.msr_bitmap);
   vmcs_read(vm_exit_ctrls.msr_store_addr);
   vmcs_read(vm_exit_ctrls.msr_load_addr);
   vmcs_read(vm_entry_ctrls.msr_load_addr);
#ifdef CONFIG_VMX_FEAT_VMCS_EXEC_PTR
   vmcs_read(vm_exec_ctrls.executive_vmcs_ptr);
#endif
   vmcs_read(vm_exec_ctrls.tsc_offset);

   if(info->vm.vmx_fx_proc.allow_1.tprs)
      vmcs_read(vm_exec_ctrls.vapic_addr);

   if(info->vm.vmx_fx_proc2.allow_1.vapic)
      vmcs_read(vm_exec_ctrls.apic_addr);

   vmcs_read(vm_exec_ctrls.eptp);
   vmcs_read(vm_exit_info.guest_physical);
   vmcs_read(vm_state.vmcs_link_ptr);

   vmcs_read(vm_exit_ctrls.exit);

   if(vm_exit_ctrls.exit.save_ia32_pat)
      vmcs_read(vm_state.ia32_pat);

   if(vm_exit_ctrls.exit.save_ia32_efer)
      vmcs_read(vm_state.ia32_efer);

   if(vm_exit_ctrls.exit.save_dbgctl)
      vmcs_read(vm_state.ia32_dbgctl);

   vmcs_read(vm_state.pdpe_0);
   vmcs_read(vm_state.pdpe_1);
   vmcs_read(vm_state.pdpe_2);
   vmcs_read(vm_state.pdpe_3);

   /* vmcs_read(vm_host_state.ia32_pat); */
   /* vmcs_read(vm_host_state.ia32_efer); */
   /* vmcs_read(vm_host_state.ia32_perf); */

   /* 32-bit fields */
   vmcs_read(vm_exec_ctrls.pin);
   vmcs_read(vm_exec_ctrls.proc);
   vmcs_read(vm_exec_ctrls.excp_bitmap);
   vmcs_read(vm_exec_ctrls.pagefault_err_code_mask);
   vmcs_read(vm_exec_ctrls.pagefault_err_code_match);
   vmcs_read(vm_exec_ctrls.cr3_target_count);

   vmcs_read(vm_exit_ctrls.msr_store_count);
   vmcs_read(vm_exit_ctrls.msr_load_count);
   vmcs_read(vm_entry_ctrls.entry);

   vmcs_read(vm_entry_ctrls.msr_load_count);
   vmcs_read(vm_entry_ctrls.int_info);
   vmcs_read(vm_entry_ctrls.err_code);
   vmcs_read(vm_entry_ctrls.insn_len);
   vmcs_read(vm_exec_ctrls.tpr_threshold);
   vmcs_read(vm_exec_ctrls.proc2);
   /* vmcs_read(vm_exec_ctrls.ple_gap); */
   /* vmcs_read(vm_exec_ctrls.ple_win); */

   vmcs_read(vm_exit_info.vmx_insn_err);
   vmcs_read(vm_exit_info.reason);
   vmcs_read(vm_exit_info.int_info);
   vmcs_read(vm_exit_info.int_err_code);
   vmcs_read(vm_exit_info.idt_info);
   vmcs_read(vm_exit_info.idt_err_code);
   vmcs_read(vm_exit_info.insn_len);
   vmcs_read(vm_exit_info.insn_info);

   vmcs_read(vm_state.es.limit);
   vmcs_read(vm_state.cs.limit);
   vmcs_read(vm_state.ss.limit);
   vmcs_read(vm_state.ds.limit);
   vmcs_read(vm_state.fs.limit);
   vmcs_read(vm_state.gs.limit);
   vmcs_read(vm_state.ldtr.limit);
   vmcs_read(vm_state.tr.limit);
   vmcs_read(vm_state.gdtr.limit);
   vmcs_read(vm_state.idtr.limit);
   vmcs_read(vm_state.es.attributes);
   vmcs_read(vm_state.cs.attributes);
   vmcs_read(vm_state.ss.attributes);
   vmcs_read(vm_state.ds.attributes);
   vmcs_read(vm_state.fs.attributes);
   vmcs_read(vm_state.gs.attributes);
   vmcs_read(vm_state.ldtr.attributes);
   vmcs_read(vm_state.tr.attributes);
   vmcs_read(vm_state.interrupt);
   vmcs_read(vm_state.activity);
#ifdef CONFIG_VMX_FEAT_VMCS_SMBASE
   vmcs_read(vm_state.smbase);
#endif
   vmcs_read(vm_state.ia32_sysenter_cs);

   if(info->vm.vmx_fx_pin.allow_1.preempt)
      vmcs_read(vm_state.preempt_timer);

   /* vmcs_read(vm_host_state.ia32_sysenter_cs); */

   /* Natural fields */
   vmcs_read(vm_exec_ctrls.cr0_mask);
   vmcs_read(vm_exec_ctrls.cr4_mask);
   vmcs_read(vm_exec_ctrls.cr0_read_shadow);
   vmcs_read(vm_exec_ctrls.cr4_read_shadow);
   vmcs_read(vm_exec_ctrls.cr3_target_0);
   vmcs_read(vm_exec_ctrls.cr3_target_1);
   vmcs_read(vm_exec_ctrls.cr3_target_2);
   vmcs_read(vm_exec_ctrls.cr3_target_3);

   vmcs_read(vm_exit_info.qualification);
   vmcs_read(vm_exit_info.io_rcx);
   vmcs_read(vm_exit_info.io_rsi);
   vmcs_read(vm_exit_info.io_rdi);
   vmcs_read(vm_exit_info.io_rip);
   vmcs_read(vm_exit_info.guest_linear);

   vmcs_read(vm_state.cr0);
   vmcs_read(vm_state.cr3);
   vmcs_read(vm_state.cr4);
   vmcs_read(vm_state.es.base);
   vmcs_read(vm_state.cs.base);
   vmcs_read(vm_state.ss.base);
   vmcs_read(vm_state.ds.base);
   vmcs_read(vm_state.fs.base);
   vmcs_read(vm_state.gs.base);
   vmcs_read(vm_state.tr.base);
   vmcs_read(vm_state.ldtr.base);
   vmcs_read(vm_state.gdtr.base);
   vmcs_read(vm_state.idtr.base);
   vmcs_read(vm_state.dr7);
   vmcs_read(vm_state.rsp);
   vmcs_read(vm_state.rip);
   vmcs_read(vm_state.rflags);
   vmcs_read(vm_state.dbg_excp);
   vmcs_read(vm_state.ia32_sysenter_esp);
   vmcs_read(vm_state.ia32_sysenter_eip);

   /* vmcs_read(vm_host_state.cr0); */
   /* vmcs_read(vm_host_state.cr3); */
   /* vmcs_read(vm_host_state.cr4); */
   /* vmcs_read(vm_host_state.fs_base); */
   /* vmcs_read(vm_host_state.gs_base); */
   /* vmcs_read(vm_host_state.tr_base); */
   /* vmcs_read(vm_host_state.gdtr_base); */
   /* vmcs_read(vm_host_state.idtr_base); */
   /* vmcs_read(vm_host_state.ia32_sysenter_esp); */
   /* vmcs_read(vm_host_state.ia32_sysenter_eip); */
   /* vmcs_read(vm_host_state.rsp); */
   /* vmcs_read(vm_host_state.rip); */

   /* Fake fields */
   vmcs_read(vm_state.cr2);
   vmcs_read(vm_state.dr6);
}

void vmx_vmcs_dirty_guest()
{
   vmcs_dirty(vm_state.es.selector);
   vmcs_dirty(vm_state.cs.selector);
   vmcs_dirty(vm_state.ss.selector);
   vmcs_dirty(vm_state.ds.selector);
   vmcs_dirty(vm_state.fs.selector);
   vmcs_dirty(vm_state.gs.selector);
   vmcs_dirty(vm_state.ldtr.selector);
   vmcs_dirty(vm_state.tr.selector);

   vmcs_dirty(vm_state.vmcs_link_ptr);
   vmcs_dirty(vm_state.ia32_dbgctl);

   if(vm_entry_ctrls.entry.load_ia32_pat)
      vmcs_dirty(vm_state.ia32_pat);

   if(vm_entry_ctrls.entry.load_ia32_efer)
      vmcs_dirty(vm_state.ia32_efer);

   if(vm_entry_ctrls.entry.load_ia32_perf)
      vmcs_dirty(vm_state.ia32_perf);

   vmcs_dirty(vm_state.pdpe_0);
   vmcs_dirty(vm_state.pdpe_1);
   vmcs_dirty(vm_state.pdpe_2);
   vmcs_dirty(vm_state.pdpe_3);

   vmcs_dirty(vm_state.es.limit);
   vmcs_dirty(vm_state.cs.limit);
   vmcs_dirty(vm_state.ss.limit);
   vmcs_dirty(vm_state.ds.limit);
   vmcs_dirty(vm_state.fs.limit);
   vmcs_dirty(vm_state.gs.limit);
   vmcs_dirty(vm_state.ldtr.limit);
   vmcs_dirty(vm_state.tr.limit);
   vmcs_dirty(vm_state.gdtr.limit);
   vmcs_dirty(vm_state.idtr.limit);
   vmcs_dirty(vm_state.es.attributes);
   vmcs_dirty(vm_state.cs.attributes);
   vmcs_dirty(vm_state.ss.attributes);
   vmcs_dirty(vm_state.ds.attributes);
   vmcs_dirty(vm_state.fs.attributes);
   vmcs_dirty(vm_state.gs.attributes);
   vmcs_dirty(vm_state.ldtr.attributes);
   vmcs_dirty(vm_state.tr.attributes);
   vmcs_dirty(vm_state.interrupt);
   vmcs_dirty(vm_state.activity);
#ifdef CONFIG_VMX_FEAT_VMCS_SMBASE
   vmcs_dirty(vm_state.smbase);
#endif
   vmcs_dirty(vm_state.ia32_sysenter_cs);

   if(info->vm.vmx_fx_pin.allow_1.preempt)
      vmcs_dirty(vm_state.preempt_timer);

   vmx_set_fixed(vm_state.cr0.low, info->vm.vmx_fx_cr0);
   vmcs_dirty(vm_state.cr0);
   vmcs_dirty(vm_state.cr3);
   vmx_set_fixed(vm_state.cr4.low, info->vm.vmx_fx_cr4);
   vmcs_dirty(vm_state.cr4);
   vmcs_dirty(vm_state.es.base);
   vmcs_dirty(vm_state.cs.base);
   vmcs_dirty(vm_state.ss.base);
   vmcs_dirty(vm_state.ds.base);
   vmcs_dirty(vm_state.fs.base);
   vmcs_dirty(vm_state.gs.base);
   vmcs_dirty(vm_state.tr.base);
   vmcs_dirty(vm_state.ldtr.base);
   vmcs_dirty(vm_state.gdtr.base);
   vmcs_dirty(vm_state.idtr.base);
   vmcs_dirty(vm_state.dr7);
   vmcs_dirty(vm_state.rsp);
   vmcs_dirty(vm_state.rip);
   vmcs_dirty(vm_state.rflags);
   vmcs_dirty(vm_state.dbg_excp);
   vmcs_dirty(vm_state.ia32_sysenter_esp);
   vmcs_dirty(vm_state.ia32_sysenter_eip);
}

void vmx_vmcs_collect_guest()
{
   vmcs_read(vm_state.es.selector);
   vmcs_read(vm_state.cs.selector);
   vmcs_read(vm_state.ss.selector);
   vmcs_read(vm_state.ds.selector);
   vmcs_read(vm_state.fs.selector);
   vmcs_read(vm_state.gs.selector);
   vmcs_read(vm_state.ldtr.selector);
   vmcs_read(vm_state.tr.selector);

   vmcs_read(vm_state.vmcs_link_ptr);
   vmcs_read(vm_state.ia32_dbgctl);

   if(vm_entry_ctrls.entry.load_ia32_pat)
      vmcs_read(vm_state.ia32_pat);

   if(vm_entry_ctrls.entry.load_ia32_efer)
      vmcs_read(vm_state.ia32_efer);

   if(vm_entry_ctrls.entry.load_ia32_perf)
      vmcs_read(vm_state.ia32_perf);

   vmcs_read(vm_state.pdpe_0);
   vmcs_read(vm_state.pdpe_1);
   vmcs_read(vm_state.pdpe_2);
   vmcs_read(vm_state.pdpe_3);

   vmcs_read(vm_state.es.limit);
   vmcs_read(vm_state.cs.limit);
   vmcs_read(vm_state.ss.limit);
   vmcs_read(vm_state.ds.limit);
   vmcs_read(vm_state.fs.limit);
   vmcs_read(vm_state.gs.limit);
   vmcs_read(vm_state.ldtr.limit);
   vmcs_read(vm_state.tr.limit);
   vmcs_read(vm_state.gdtr.limit);
   vmcs_read(vm_state.idtr.limit);
   vmcs_read(vm_state.es.attributes);
   vmcs_read(vm_state.cs.attributes);
   vmcs_read(vm_state.ss.attributes);
   vmcs_read(vm_state.ds.attributes);
   vmcs_read(vm_state.fs.attributes);
   vmcs_read(vm_state.gs.attributes);
   vmcs_read(vm_state.ldtr.attributes);
   vmcs_read(vm_state.tr.attributes);
   vmcs_read(vm_state.interrupt);
   vmcs_read(vm_state.activity);
#ifdef CONFIG_VMX_FEAT_VMCS_SMBASE
   vmcs_read(vm_state.smbase);
#endif
   vmcs_read(vm_state.ia32_sysenter_cs);

   if(info->vm.vmx_fx_pin.allow_1.preempt)
      vmcs_read(vm_state.preempt_timer);

   vmx_set_fixed(vm_state.cr0.low, info->vm.vmx_fx_cr0);
   vmcs_read(vm_state.cr0);
   vmcs_read(vm_state.cr3);
   vmx_set_fixed(vm_state.cr4.low, info->vm.vmx_fx_cr4);
   vmcs_read(vm_state.cr4);
   vmcs_read(vm_state.es.base);
   vmcs_read(vm_state.cs.base);
   vmcs_read(vm_state.ss.base);
   vmcs_read(vm_state.ds.base);
   vmcs_read(vm_state.fs.base);
   vmcs_read(vm_state.gs.base);
   vmcs_read(vm_state.tr.base);
   vmcs_read(vm_state.ldtr.base);
   vmcs_read(vm_state.gdtr.base);
   vmcs_read(vm_state.idtr.base);
   vmcs_read(vm_state.dr7);
   vmcs_read(vm_state.rsp);
   vmcs_read(vm_state.rip);
   vmcs_read(vm_state.rflags);
   vmcs_read(vm_state.dbg_excp);
   vmcs_read(vm_state.ia32_sysenter_esp);
   vmcs_read(vm_state.ia32_sysenter_eip);
}
