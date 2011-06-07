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
#include <svm_exit_fail.h>
#include <svm_exit.h>
#include <svm_vmcb.h>
#include <svm_vm.h>
#include <excp.h>
#include <string.h>
#include <disasm.h>
#include <debug.h>
#include <gdb.h>
#include <info_data.h>

extern info_data_t *info;

extern char* exception_names[];

static char* svm_vmexit_resolver_string[] = {
   "Physical Interrupt",
   "Physical NMI",
   "Physical SMI",
   "Physical INIT",
   "Virtual Interrupt",
   "CR0 write selective",
   "Read IDTR",
   "Read GDTR",
   "Read LDTR",
   "Read TR",
   "Write IDTR",
   "Write GDTR",
   "Write LDTR",
   "Write TR",
   "RDTSC",
   "RDPMC",
   "PUSHF",
   "POPF",
   "CPUID",
   "RSM",
   "IRET",
   "Software Interrupt",
   "INVD",
   "PAUSE",
   "HALT",
   "INVLPG",
   "INVLPGA",
   "I/O access",
   "MSR access",
   "Task Switch",
   "FPU error",
   "Shutdown",
   "VMRUN",
   "VMMCALL",
   "VMLOAD",
   "VMSAVE",
   "STGI",
   "CLGI",
   "SKINIT",
   "RDTSCP",
   "ICEBP",
   "WBINVD",
   "MONITOR",
   "MWAIT",
   "MWAIT_COND"
};

static char* svm_vmexit_interrupt_info_string[] = {
   "External or Virtual Interrupt",
   "Reserved",
   "NMI",
   "Exception",
   "Software Interrupt"
};

static void svm_vmexit_show_gpr()
{
   vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area;

   debug(SVM,
	 "-\n"
	 "eax = 0x%x eax = 0x%x (vmcb)\n"
	 "ebx = 0x%x ecx = 0x%x\n"
	 "edx = 0x%x ebp = 0x%x\n"
	 "esi = 0x%x edi = 0x%x\n"
	 "eip = 0x%x esp = 0x%x\n"
	 "dr6 = 0x%x dr7 = 0x%x\n"
	 ,info->vm.cpu.gpr->rax.low
	 ,info->vm.cpu.vmc->vm_vmcb.state_area.rax.low
	 ,info->vm.cpu.gpr->rbx.low
	 ,info->vm.cpu.gpr->rcx.low
	 ,info->vm.cpu.gpr->rdx.low
	 ,info->vm.cpu.gpr->rbp.low
	 ,info->vm.cpu.gpr->rsi.low
	 ,info->vm.cpu.gpr->rdi.low
	 ,state->rip.low,state->rsp.low
	 ,state->dr6.raw,state->dr7.raw);
}

static void svm_vmexit_show_cr()
{
   vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area;
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;

   debug(SVM,
	 "-\n"
	 "cpl             : %d\n"
	 "cr0             : 0x%x\n"
	 "cr2             : 0x%X\n"
	 "cr3             : 0x%X\n"
	 "cr4             : 0x%x\n"
	 "tpr             : 0x%x\n"
	 "eflags          : 0x%x (vm:%d rf:%d iopl:%d if:%d tf:%d)\n"
	 "efer            : 0x%x (lma:%d lme:%d)\n"
	 "gdtr (limit)    : 0x%x (0x%x)\n"
	 "idtr (limit)    : 0x%x (0x%x)\n"
	 ,state->cpl
	 ,state->cr0.low
	 ,state->cr2.raw
	 ,state->cr3.raw
	 ,state->cr4.low
	 ,ctrls->int_ctrl.v_tpr
	 ,state->rflags.low,state->rflags.vm
	 ,state->rflags.rf,state->rflags.iopl
	 ,state->rflags.IF,state->rflags.tf
	 ,state->efer.eax, state->efer.lma, state->efer.lme
	 ,state->gdtr.base_addr.low, state->gdtr.limit.raw
	 ,state->idtr.base_addr.low, state->idtr.limit.raw);
}

static void svm_vmexit_show_segment(vmcb_segment_desc_t *seg, char *name)
{
   debug(SVM,
	 "-\n"
	 "%s.base (limit) : 0x%x (0x%x)\n"
	 "%s.selector     : 0x%x (idx:%d rpl:%d ti:%d)\n"
	 "%s.access       : dpl:0x%x type:0x%x l:%d d:%d g:%d p:%d\n"
	 ,name,seg->base_addr.low,seg->limit.raw
	 ,name,seg->selector.raw
	 ,seg->selector.index,seg->selector.rpl,seg->selector.ti
	 ,name,seg->attributes.dpl,seg->attributes.type
	 ,seg->attributes.l,seg->attributes.d
	 ,seg->attributes.g,seg->attributes.p);
}

static void svm_vmexit_show_segments()
{
   vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area;

   svm_vmexit_show_segment(&state->cs, "cs");
   svm_vmexit_show_segment(&state->ss, "ss");
   svm_vmexit_show_segment(&state->ds, "ds");
   svm_vmexit_show_segment(&state->es, "es");
   svm_vmexit_show_segment(&state->fs, "fs");
   svm_vmexit_show_segment(&state->gs, "gs");
   svm_vmexit_show_segment(&state->tr, "tr");
}

static void svm_vmexit_show_insn()
{
   ud_t disasm;

   if(disassemble(&disasm))
      debug(SVM,
	    "-\ninsn            : \"%s\" (len %d)\n"
	    ,ud_insn_asm(&disasm),ud_insn_len(&disasm));
}

static void svm_vmexit_show_excp()
{
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;
   uint32_t          n = ctrls->exit_code.low - SVM_VMEXIT_EXCP_START;

   debug(SVM,"-\nexception : vector 0x%x err_code 0x%x\n",n,ctrls->exit_info_1.low);
   if(n == GP_EXCP)
   {
      gp_err_t e; e.raw = ctrls->exit_info_1.low;
      debug(SVM,
	    "#GP error : idx 0x%x ti %d idt %d ext %d\n"
	    ,e.idx, e.ti, e.idt, e.ext);
   }
}

static void svm_vmexit_show_event()
{
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;

   if(range(ctrls->exit_code.low, SVM_VMEXIT_EXCP_START, SVM_VMEXIT_EXCP_END))
      svm_vmexit_show_excp();
   else if(ctrls->exit_code.low == SVM_VMEXIT_INTR ||
	   ctrls->exit_code.low == SVM_VMEXIT_SWINT)
   {
      debug(SVM,
	    "-\ninterrupt : tpr 0x%x irq %d prio %d ign %d mask %d vector 0x%x\n"
	    ,ctrls->int_ctrl.v_tpr
	    ,ctrls->int_ctrl.v_irq
	    ,ctrls->int_ctrl.v_intr_prio
	    ,ctrls->int_ctrl.v_ign_tpr
	    ,ctrls->int_ctrl.v_intr_masking
	    ,ctrls->int_ctrl.v_intr_vector
	 );
   }
   else if(ctrls->exit_code.low == SVM_VMEXIT_SMI)
   {
      debug(SVM, "-\n%s SMI raised\n",
	    ctrls->exit_info_1.smi.smi_rc?"External":"Internal");

      if(ctrls->exit_info_1.smi.val)
      {
	 debug(SVM,
	       "----> asserted during i/o\n"
	       "i/o insn rip   : 0x%x\n"
	       "acc sz         : %d byte(s)\n"
	       "direction      : %s\n"
	       "string insn    : %s\n"
	       "rep prefix     : %s\n"
	       "port           : 0x%x\n"
	       "RFLAGS.TF      : %d\n"
	       "brk            : 0x%x\n"
	       ,ctrls->exit_info_2.low
	       ,(ctrls->exit_info_1.high>>4) & 0x7
	       ,ctrls->exit_info_1.smi.d?"in":"out"
	       ,ctrls->exit_info_1.smi.s?"yes":"no"
	       ,ctrls->exit_info_1.smi.rep?"yes":"no"
	       ,ctrls->exit_info_1.smi.port
	       ,ctrls->exit_info_1.smi.tf
	       ,ctrls->exit_info_1.smi.brk);
      }
   }
   else if(ctrls->exit_code.low == SVM_VMEXIT_IOIO)
   {
      svm_io_t *io = (svm_io_t*)&ctrls->exit_info_1;

      debug(SVM,
	    "-\n i/o  : d %d s %d rep %d port 0x%x\n"
	    ,io->io.d,io->io.s
	    ,io->io.rep,io->io.port);
   }
}

static void svm_vmexit_show_deliver()
{
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;

   if(ctrls->exit_int_info.v)
   {
      char *str;

      if(ctrls->exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_EXCP)
	 str = exception_names[ctrls->exit_int_info.vector];
      else
	 str = svm_vmexit_interrupt_info_string[ctrls->exit_int_info.type];

      debug(SVM,
	    "-\nidt delivery : %s vector 0x%x err_code 0x%x\n"
	    ,str,ctrls->exit_int_info.vector,ctrls->exit_int_info.err_code);
   }
}

static void svm_vmexit_str(char *buffer, size_t size, uint32_t err)
{
   char   *name;
   size_t sz;

   if(!buffer || !size)
      return;

   if(err < SVM_VMEXIT_EXCP_START)
   {
      uint32_t _err = (err<SVM_VMEXIT_DR_READ_START)?err:(err - 32);
      snprintf(buffer, size, "%s %cR%d",
		_err<SVM_VMEXIT_CR_WRITE_START?"read from":"write to",
		err<SVM_VMEXIT_DR_READ_START?'C':'D', err%16);
   }
   else
   {
      if(err < SVM_VMEXIT_FIRST_RESOLVER)
	 name = exception_names[err - SVM_VMEXIT_EXCP_START];
      else if(err <= SVM_VMEXIT_LAST_RESOLVER)
	 name = svm_vmexit_resolver_string(err);
      else if(err == SVM_VMEXIT_NPF)
	 name = "Nested Page Fault";
      else if(err == (typeof(err))SVM_VMEXIT_INVALID)
	 name = "Invalid Guest State";

      sz = min(strlen(name), size-1);
      memcpy(buffer, name, sz);
      buffer[sz] = 0;
   }
}

static void svm_vmexit_show_basic()
{
   char exit_str[64];
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;

   svm_vmexit_str(exit_str, sizeof(exit_str), ctrls->exit_code.low);

   debug(SVM,
	 "\n         <------------------- VM-EXIT ------------------->\n\n"
	 "reason          : %s (%d)\n"
	 "info 1          : 0x%X\n"
	 "info 2          : 0x%X\n"
	 ,exit_str,ctrls->exit_code.low
	 ,ctrls->exit_info_1.raw
	 ,ctrls->exit_info_2.raw);
}

static void svm_vmexit_show()
{
   svm_vmexit_show_basic();
   svm_vmexit_show_insn();
   svm_vmexit_show_gpr();
   svm_vmexit_show_cr();
   svm_vmexit_show_segments();
   svm_vmexit_show_event();
   svm_vmexit_show_deliver();

   debug(SVM,"\n         <----------------------------------------------->\n\n");
}

void svm_vmexit_failure()
{
   svm_vmexit_show();
   while(1)
      ctrl_logic();
}

