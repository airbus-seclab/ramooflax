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
#include <vmx_exit_fail.h>
#include <vmx_exit_invg.h>
#include <vmx_exit.h>
#include <vmx_vmcs.h>
#include <vmx_vm.h>
#include <show.h>
#include <excp.h>
#include <string.h>
#include <disasm.h>
#include <gdbstub.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

char vmx_vmresume_failure_fmt[] = "failed (%d)\n";
char vmx_vmresume_failure_fnm[] = "vmresume";

extern char* exception_names[];

static char* vmx_vmexit_resolved_string[] = {
   "Exception or NMI",
   "External Interrupt",
   "Triple Fault",
   "INIT signal",
   "Start-up IPI",
   "I/O SMI",
   "Other SMI",
   "Interrupt window",
   "NMI window",
   "Task switch",
   "CPUID",
   "GETSEC",
   "HLT",
   "INVD",
   "INVLPG",
   "RDPMC",
   "RDTSC",
   "RSM",
   "VMCALL",
   "VMCLEAR",
   "VMLAUNCH",
   "VMPTRLD",
   "VMPTRST",
   "VMREADA",
   "VMRESUME",
   "VMWRITE",
   "VMXOFF",
   "VMXON",
   "Control Register Access",
   "Debug Register Access",
   "I/O Insn",
   "RDMSR",
   "WRMSR",
   "Invalid Guest State (VM-entry)",
   "MSR Loading (VM-entry)",
   "-- Undefined --",
   "MWAIT",
   "MTF",
   "-- Undefined --",
   "MONITOR",
   "PAUSE",
   "Machine-Check (VM-entry)",
   "-- Undefined --",
   "TPR threshold",
   "APIC",
   "-- Undefined --",
   "DTR",
   "LDTR",
   "EPT violation",
   "EPT misconfiguration",
   "INVEPT",
   "RDTSCP",
   "Preempt Timer",
   "INVVPID",
   "WBINVD",
   "XSETBV",
};

static void vmx_vmexit_show_cap_fix()
{
   vmx_show_basic_info();
   vmx_show_misc_data();
   vmx_show_fixed_pin_ctls();
   vmx_show_pin_ctls();
   vmx_show_fixed_proc_ctls();
   vmx_show_proc_ctls();
   vmx_show_fixed_proc2_ctls();
   vmx_show_proc2_ctls();
   vmx_show_fixed_entry_ctls();
   vmx_show_entry_ctls();
   vmx_show_fixed_exit_ctls();
   vmx_show_exit_ctls();
   vmx_show_ept_cap();
}

static void vmx_vmexit_show_non_reg_state()
{
   printf("\n- non register state\n"
          "activity  state  : %d\n"
          "interrupt state  : sti %d movss %d smi %d nmi %d (0x%x)\n"
          "pending   #DB    : b0 %d b1 %d b2 %d b3 %d be %d bs %d (0x%X)\n"
          "vmcs link ptr    : 0x%X\n"
          "preempt timer    : 0x%x\n"
          "pdpe(s)          : 0x%X 0x%X 0x%X 0x%X\n"
          ,vm_state.activity.raw
          ,vm_state.interrupt.sti,vm_state.interrupt.mss
          ,vm_state.interrupt.smi,vm_state.interrupt.nmi
          ,vm_state.interrupt.raw
          ,vm_state.dbg_excp.b0,vm_state.dbg_excp.b1,vm_state.dbg_excp.b2
          ,vm_state.dbg_excp.b3,vm_state.dbg_excp.be,vm_state.dbg_excp.bs
          ,vm_state.dbg_excp.raw
          ,vm_state.vmcs_link_ptr.raw,vm_state.preempt_timer.raw
          ,vm_state.pdpe_0.raw,vm_state.pdpe_1.raw
          ,vm_state.pdpe_2.raw,vm_state.pdpe_3.raw);
}

static void vmx_vmexit_show_gpr()
{
   printf("\n- general registers\n"
          "rip     : 0x%X\n"
          "eflags  : 0x%x (vm:%d rf:%d iopl:%d if:%d tf:%d)\n"
          "rax     : 0x%X\n"
          "rbx     : 0x%X\n"
          "rcx     : 0x%X\n"
          "rdx     : 0x%X\n"
          "rsi     : 0x%X\n"
          "rdi     : 0x%X\n"
          "rsp     : 0x%X\n"
          "rbp     : 0x%X\n"
          ,vm_state.rip.raw
          ,vm_state.rflags.low
          ,vm_state.rflags.vm,vm_state.rflags.rf,vm_state.rflags.iopl
          ,vm_state.rflags.IF,vm_state.rflags.tf
          ,info->vm.cpu.gpr->rax.raw
          ,info->vm.cpu.gpr->rbx.raw
          ,info->vm.cpu.gpr->rcx.raw
          ,info->vm.cpu.gpr->rdx.raw
          ,info->vm.cpu.gpr->rsi.raw
          ,info->vm.cpu.gpr->rdi.raw
          ,vm_state.rsp.raw
          ,info->vm.cpu.gpr->rbp.raw);
}

static void vmx_vmexit_show_cr()
{
   printf("\n- control registers\n"
          "mode            : %d bits\n"
          "cpl             : %d\n"
          "cr0             : 0x%x (pe:%d pg:%d ne:%d)\n"
          "cr0 fixed       : 0x%x 0x%x\n"
          "cr2             : 0x%X\n"
          "cr3             : 0x%X\n"
          "cr4             : 0x%x (pae:%d pse:%d pge:%d vmxe:%d)\n"
          "cr4 fixed       : 0x%x 0x%x\n"
          "dr6             : 0x%X\n"
          "dr7             : 0x%X\n"
          "efer (shadow)   : 0x%x (lma:%d lme:%d nxe:%d)\n"
          "efer            : 0x%x (lma:%d lme:%d nxe:%d ia32e:%d)\n"
          "gdtr (limit)    : 0x%x (0x%x)\n"
          "idtr (limit)    : 0x%x (0x%x)\n"
          ,cpu_addr_sz(), __cpl
          ,vm_state.cr0.low, vm_state.cr0.pe, vm_state.cr0.pg, vm_state.cr0.ne
          ,info->vm.vmx_fx_cr0.allow_0.raw, info->vm.vmx_fx_cr0.allow_1.raw
          ,vm_state.cr2.raw
          ,vm_state.cr3.raw
          ,vm_state.cr4.low, vm_state.cr4.pae
          ,vm_state.cr4.pse, vm_state.cr4.pge, vm_state.cr4.vmxe
          ,info->vm.vmx_fx_cr4.allow_0.raw, info->vm.vmx_fx_cr4.allow_1.raw
          ,vm_state.dr6.raw
          ,vm_state.dr7.raw
          ,info->vm.efer.eax, info->vm.efer.lma
          ,info->vm.efer.lme, info->vm.efer.nxe
          ,vm_state.ia32_efer.eax, vm_state.ia32_efer.lma
          ,vm_state.ia32_efer.lme, vm_state.ia32_efer.nxe
          ,vm_entry_ctrls.entry.ia32e
          ,vm_state.gdtr.base.low, vm_state.gdtr.limit.raw
          ,vm_state.idtr.base.low, vm_state.idtr.limit.raw);
}

static void vmx_vmexit_show_segment(vmcs_guest_seg_desc_t *seg, char *name)
{
   printf("-\n"
          "%s.base (limit) : 0x%x (0x%x)\n"
          "%s.selector     : 0x%x (idx:%d rpl:%d ti:%d)\n"
          "%s.access       : 0x%x (dpl:0x%x type:0x%x l:%d d:%d g:%d p:%d)\n"
          ,name,seg->base.low,seg->limit.raw
          ,name,seg->selector.raw
          ,seg->selector.index,seg->selector.rpl,seg->selector.ti
          ,name,seg->attributes.raw
          ,seg->attributes.dpl,seg->attributes.type
          ,seg->attributes.l,seg->attributes.d
          ,seg->attributes.g,seg->attributes.p);
}

static void vmx_vmexit_show_segments()
{
   printf("\n- segment registers\n");
   vmx_vmexit_show_segment(&vm_state.cs, "cs");
   vmx_vmexit_show_segment(&vm_state.ss, "ss");
   vmx_vmexit_show_segment(&vm_state.ds, "ds");
   vmx_vmexit_show_segment(&vm_state.es, "es");
   vmx_vmexit_show_segment(&vm_state.fs, "fs");
   vmx_vmexit_show_segment(&vm_state.gs, "gs");
   vmx_vmexit_show_segment(&vm_state.tr, "tr");
}

static void vmx_vmexit_show_insn()
{
   ud_t disasm;

   if(disassemble(&disasm) == VM_DONE)
      printf("\n- insn : \"%s\" (len %d)\n"
             ,ud_insn_asm(&disasm),ud_insn_len(&disasm));
}

char* vmx_vmexit_string_from_vector_type(uint8_t type,  uint8_t vector)
{
   switch(type)
   {
   case VMCS_EVT_INFO_TYPE_HW_EXCP: return exception_names[vector];
   case VMCS_EVT_INFO_TYPE_HW_INT:  return "hard int";
   case VMCS_EVT_INFO_TYPE_SW_INT:  return "soft int";
   case VMCS_EVT_INFO_TYPE_NMI:     return "nmi";
   }

   return "(unknown)";
}

static void vmx_vmexit_show_gp_event()
{
   int_desc_t *idt;
   pg_wlk_t   wlk;

   if(!vm_exit_info.int_err_code.sl.idt ||_xx_lmode())
      return;

   idt = (int_desc_t*)(vm_state.idtr.base.raw & 0xffffffff);

   if(!__paging())
      goto __show_gp;
   else if(vm_full_walk((offset_t)idt, &wlk))
   {
      idt = (int_desc_t*)wlk.addr;
      goto __show_gp;
   }

   printf("#GP related to IDT and IDT is no mapped\n");
   return;

__show_gp:
   printf("#GP related to IDT entry 0x%x [0x%X]\n"
          ,vm_exit_info.int_err_code.sl.idx
          ,idt[vm_exit_info.int_err_code.sl.idx].raw);
}

static void vmx_vmexit_show_event()
{
   char *name;

   if(!vm_exit_info.int_info.v)
      return;

   name = vmx_vmexit_string_from_vector_type(vm_exit_info.int_info.type,
                                             vm_exit_info.int_info.vector);
   printf("\n- event : %s (type %d) vector 0x%x err_code 0x%x\n"
          ,name,vm_exit_info.int_info.type
          ,vm_exit_info.int_info.vector
          ,vm_exit_info.int_info.v_err?vm_exit_info.int_err_code.raw:0);

   if(vm_exit_info.int_info.type != VMCS_EVT_INFO_TYPE_HW_EXCP)
      return;

   switch(vm_exit_info.int_info.vector)
   {
   case PF_EXCP:
   {
      pg_wlk_t wlk;
      vm_full_walk(__cr2.raw, &wlk);
      printf("cr2 0x%X -> nested 0x%X\n", __cr2.raw, wlk.addr);
      break;
   }
   case GP_EXCP:
   {
      vmx_vmexit_show_gp_event();
      break;
   }
   }
}

static void vmx_vmexit_show_deliver()
{
   char *name;

   if(vm_entry_ctrls.int_info.v)
   {
      name = vmx_vmexit_string_from_vector_type(vm_entry_ctrls.int_info.type,
                                                vm_entry_ctrls.int_info.vector);
      printf("\n- entry event : %s (type %d) vector 0x%x err_code 0x%x\n"
             ,name
             ,vm_entry_ctrls.int_info.type
             ,vm_entry_ctrls.int_info.vector
             ,vm_entry_ctrls.int_info.dec?vm_entry_ctrls.err_code.raw:0);
   }

   if(vm_exit_info.idt_info.v)
   {
      name = vmx_vmexit_string_from_vector_type(vm_exit_info.idt_info.type,
                                                vm_exit_info.idt_info.vector);
      printf("\n- idt delivery : %s (type %d) vector 0x%x err_code 0x%x\n"
             ,name
             ,vm_exit_info.idt_info.type
             ,vm_exit_info.idt_info.vector
             ,vm_exit_info.idt_info.v_err?vm_exit_info.idt_err_code.raw:0);
   }
}

static void vmx_vmexit_show_info()
{
   printf("qualification   : 0x%X\n", vm_exit_info.qualification.raw);

   if(vm_exit_info.reason.basic == VMX_VMEXIT_EPT_CONF)
   {
      offset_t  vaddr;
      pg_wlk_t  wlk;
      int       mode;

      vm_get_code_addr(&vaddr, 0, &mode);
      npg_walk(vaddr, &wlk);
   }
   else if(vm_exit_info.reason.basic == VMX_VMEXIT_EPT)
   {
      pg_wlk_t wlk;

      printf("-\n"
             " . glinear      : 0x%X\n"
             " . gphysical    : 0x%X\n"
             ,vm_exit_info.guest_linear.raw
             ,vm_exit_info.guest_physical.raw
         );
      npg_walk(vm_exit_info.guest_physical.raw, &wlk);
   }
   else if(vm_exit_info.reason.basic == VMX_VMEXIT_CR_ACCESS)
   {
      printf("-\n"
             " . cr num       : %d\n"
             " . type         : %d\n"
             " . lmsw op      : %d\n"
             " . gpr          : %d\n"
             " . lmsw data    : 0x%x\n"
             ,vm_exit_info.qualification.cr.nr
             ,vm_exit_info.qualification.cr.type
             ,vm_exit_info.qualification.cr.lmsw_op
             ,vm_exit_info.qualification.cr.gpr
             ,vm_exit_info.qualification.cr.lmsw_data);
   }
   else if(vm_exit_info.reason.basic == VMX_VMEXIT_IO_INSN)
   {
      printf("-\n"
             " . acc sz       : %d byte(s)\n"
             " . direction    : %s\n"
             " . string insn  : %s\n"
             " . rep prefix   : %s\n"
             " . operand      : %s\n"
             " . port         : 0x%x\n"
             ,vm_exit_info.qualification.io.sz+1
             ,vm_exit_info.qualification.io.d?"in":"out"
             ,vm_exit_info.qualification.io.s?"yes":"no"
             ,vm_exit_info.qualification.io.rep?"yes":"no"
             ,vm_exit_info.qualification.io.op?"imm":"dx"
             ,vm_exit_info.qualification.io.port);
   }
   else if(vm_exit_info.reason.basic == VMX_VMEXIT_INVL_G_STATE)
      vmx_vmexit_show_invalid_guest();
}

static void vmx_vmexit_show_detail()
{
   vmx_vmexit_show_info();
   vmx_vmexit_show_insn();
   vmx_vmexit_show_non_reg_state();
   vmx_vmexit_show_gpr();
   vmx_vmexit_show_cr();
   vmx_vmexit_show_segments();
   vmx_vmexit_show_event();
   vmx_vmexit_show_deliver();
}

static int vmx_vmexit_show_exit()
{
   char *name;
   int  rc = 1;

   if(vm_exit_info.reason.basic < VMX_VMEXIT_RESOLVERS_NR)
      name = vmx_vmexit_resolved_string[vm_exit_info.reason.basic];
   else
   {
      name = "Invalid VM-EXIT value";
      rc = 0;
   }

   printf("\n         <------------------- VM-EXIT ------------------->\n"
          "reason          : %s (%d)\n"
          "vm-exit-count   : 0x%X\n"
          "vm-entry fail   : %s\n"
          "vmx-root        : %s\n"
          "pending MTF     : %s\n"
          ,name
          ,vm_exit_info.reason.basic
          ,info->vmm.ctrl.vmexit_cnt.raw
          ,vm_exit_info.reason.entry?"yes":"no"
          ,vm_exit_info.reason.root?"yes":"no"
          ,vm_exit_info.reason.mtf?"yes":"no");

   return rc;
}

void vmx_vmexit_show()
{
   vmx_vmcs_collect();

   if(vmx_vmexit_show_exit())
      vmx_vmexit_show_detail();

   printf("\n         <----------------- VMX Features ---------------->\n");
   show_vmm_mem_map();
   vmx_vmexit_show_cap_fix();
}

void vmx_vmexit_failure()
{
   vmx_vmexit_show();
#ifdef CONFIG_GDBSTUB
   while(1) gdbstub();
#else
   lock_vmm();
#endif
}

void __regparm__(1) vmx_vmresume_failure(vmx_insn_err_t vmx_err)
{
   panic("vmresume failed (%d)\n", vmx_err.raw);
}
