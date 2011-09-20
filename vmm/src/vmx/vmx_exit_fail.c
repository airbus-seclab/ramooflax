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
#include <vmx_exit_fail.h>
#include <vmx_exit.h>
#include <vmx_vmcs.h>
#include <vmx_vm.h>
#include <excp.h>
#include <string.h>
#include <disasm.h>
#include <debug.h>
#include <gdb.h>
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

static void vmx_vmexit_show_gpr()
{
   printf("-\n"
	  "eip = 0x%x\n"
	  "esp = 0x%x ebp = 0x%x\n"
	  "eax = 0x%x ebx = 0x%x\n"
	  "ecx = 0x%x edx = 0x%x\n"
	  "esi = 0x%x edi = 0x%x\n"
	  "dr6 = 0x%X dr7 = 0x%X\n"
	  ,vm_state.rip.low
	  ,vm_state.rsp.low
	  ,info->vm.cpu.gpr->rbp.low
	  ,info->vm.cpu.gpr->rax.low
	  ,info->vm.cpu.gpr->rbx.low
	  ,info->vm.cpu.gpr->rcx.low
	  ,info->vm.cpu.gpr->rdx.low
	  ,info->vm.cpu.gpr->rsi.low
	  ,info->vm.cpu.gpr->rdi.low
	  ,vm_state.dr6.raw
	  ,vm_state.dr7.raw);
}

static void vmx_vmexit_show_cr()
{
   printf("-\n"
	  "cpl             : %d\n"
	  "cr0             : 0x%x\n"
	  "cr2             : 0x%X\n"
	  "cr3             : 0x%X\n"
	  "cr4             : 0x%x\n"
	  "eflags          : 0x%x (vm:%d rf:%d iopl:%d if:%d tf:%d)\n"
	  "efer            : 0x%x (lma:%d lme:%d)\n"
	  "gdtr (limit)    : 0x%x (0x%x)\n"
	  "idtr (limit)    : 0x%x (0x%x)\n"
	  ,__cpl
	  ,vm_state.cr0.low
	  ,vm_state.cr2.raw
	  ,vm_state.cr3.raw
	  ,vm_state.cr4.low
	  ,vm_state.rflags.low,vm_state.rflags.vm
	  ,vm_state.rflags.rf,vm_state.rflags.iopl
	  ,vm_state.rflags.IF,vm_state.rflags.tf
	  ,vm_state.ia32_efer.eax, vm_state.ia32_efer.lma, vm_state.ia32_efer.lme
	  ,vm_state.gdtr.base.low, vm_state.gdtr.limit.raw
	  ,vm_state.idtr.base.low, vm_state.idtr.limit.raw);
}

static void vmx_vmexit_show_segment(vmcs_guest_seg_desc_t *seg, char *name)
{
   printf("-\n"
	  "%s.base (limit) : 0x%x (0x%x)\n"
	  "%s.selector     : 0x%x (idx:%d rpl:%d ti:%d)\n"
	  "%s.access       : dpl:0x%x type:0x%x l:%d d:%d g:%d p:%d\n"
	  ,name,seg->base.low,seg->limit.raw
	  ,name,seg->selector.raw
	  ,seg->selector.index,seg->selector.rpl,seg->selector.ti
	  ,name,seg->attributes.dpl,seg->attributes.type
	  ,seg->attributes.l,seg->attributes.d
	  ,seg->attributes.g,seg->attributes.p);
}

static void vmx_vmexit_show_segments()
{
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

   if(disassemble(&disasm))
      printf("-\ninsn            : \"%s\" (len %d)\n"
	     ,ud_insn_asm(&disasm),ud_insn_len(&disasm));
}

char* vmx_vmexit_string_from_vector_type(uint8_t type,  uint8_t vector)
{
   switch(type)
   {
   case VMCS_IDT_INFO_TYPE_HW_EXCP: return exception_names[vector];
   case VMCS_IDT_INFO_TYPE_HW_INT:  return "hard int";
   case VMCS_IDT_INFO_TYPE_SW_INT:  return "soft int";
   case VMCS_IDT_INFO_TYPE_NMI:     return "nmi";
   }

   return "(unknown)";
}

static void vmx_vmexit_show_event()
{
   if(vm_exit_info.int_info.v)
   {
      char *name = vmx_vmexit_string_from_vector_type(vm_exit_info.int_info.type,
						      vm_exit_info.int_info.vector);
      printf("-\nevent           : %s (%d) vector 0x%x err_code 0x%x\n"
	     ,name,vm_exit_info.int_info.type
	     ,vm_exit_info.int_info.vector
	     ,vm_exit_info.int_info.v_err?vm_exit_info.int_err_code.raw:0);
   }
}

static void vmx_vmexit_show_deliver()
{
   if(vm_exit_info.idt_info.v)
   {
      char *name = vmx_vmexit_string_from_vector_type(vm_exit_info.idt_info.type,
						      vm_exit_info.idt_info.vector);
      printf("-\nidt delivery    : %s (%d) vector 0x%x err_code 0x%x\n"
	     ,name,vm_exit_info.idt_info.type
	     ,vm_exit_info.idt_info.vector
	     ,vm_exit_info.idt_info.v_err?vm_exit_info.idt_err_code.raw:0);
   }
}

static int vmx_vmexit_show_basic()
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

   printf("\n         <------------------- VM-EXIT ------------------->\n\n"
	  "reason          : %s (%d)\n"
	  "vm-entry fail   : %s\n"
	  "vmx-root        : %s\n"
	  "pending MTF     : %s\n"
	  ,name, vm_exit_info.reason.basic
	  ,vm_exit_info.reason.entry?"yes":"no"
	  ,vm_exit_info.reason.root?"yes":"no"
	  ,vm_exit_info.reason.mtf?"yes":"no");

   return rc;
}

static void vmx_vmexit_show_info()
{
   printf("qualification   : 0x%X\n", vm_exit_info.qualification.raw);

   if(vm_exit_info.reason.basic == VMX_VMEXIT_EPT_CONF)
   {
      offset_t vaddr, paddr;
      int      mode;

      vm_get_code_addr(&vaddr, 0, &mode);
      npg_walk(vaddr, &paddr);
   }
   else if(vm_exit_info.reason.basic == VMX_VMEXIT_EPT)
   {
      offset_t paddr;

      printf("-\n"
	     " . glinear      : 0x%X\n"
	     " . gphysical    : 0x%X\n"
	     ,vm_exit_info.guest_linear.raw
	     ,vm_exit_info.guest_physical.raw
	 );
      npg_walk(vm_exit_info.guest_physical.raw, &paddr);
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
   {
      switch(vm_exit_info.qualification.low)
      {
      case 2: printf("page directory register related load failure\n");break;
      case 3: printf("NMI injection failure\n");break;
      case 4: printf("invalid VMCS link pointer\n");break;
      default:printf("(unused) %d\n", vm_exit_info.qualification.low);break;
      }
   }
}

static void vmx_vmexit_show_all()
{
   vmx_vmcs_collect();
   vmx_vmexit_show_info();
   vmx_vmexit_show_insn();
   vmx_vmexit_show_gpr();
   vmx_vmexit_show_cr();
   vmx_vmexit_show_segments();
   vmx_vmexit_show_event();
   vmx_vmexit_show_deliver();
}

void vmx_vmexit_show()
{
   if(vmx_vmexit_show_basic())
      vmx_vmexit_show_all();
}

void vmx_vmexit_failure()
{
   vmx_vmexit_show();
   while(1)
      ctrl_logic();
}

void __regparm__(1) vmx_vmresume_failure(vmx_insn_err_t vmx_err)
{
   panic("vmresume failed (%d)\n", vmx_err.raw);
}
