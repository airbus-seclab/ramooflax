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
#include <svm_exit_fail.h>
#include <svm_exit.h>
#include <svm_vmcb.h>
#include <svm_vm.h>
#include <excp.h>
#include <string.h>
#include <disasm.h>
#include <gdbstub.h>
#include <debug.h>
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
   printf("-\n"
          "eip = 0x%X eax = 0x%X (vmcb)\n"
          "esp = 0x%X ebp = 0x%X\n"
          "eax = 0x%X ebx = 0x%X\n"
          "ecx = 0x%X edx = 0x%X\n"
          "esi = 0x%X edi = 0x%X\n"
          "dr6 = 0x%X dr7 = 0x%X\n"
          ,vm_state.rip.raw
          ,vm_state.rax.raw
          ,vm_state.rsp.raw
          ,info->vm.cpu.gpr->rbp.raw
          ,info->vm.cpu.gpr->rax.raw
          ,info->vm.cpu.gpr->rbx.raw
          ,info->vm.cpu.gpr->rcx.raw
          ,info->vm.cpu.gpr->rdx.raw
          ,info->vm.cpu.gpr->rsi.raw
          ,info->vm.cpu.gpr->rdi.raw
          ,vm_state.dr6.raw
          ,vm_state.dr7.raw);
}

static void svm_vmexit_show_cr()
{
   printf("-\n"
          "cpl             : %d\n"
          "cr0             : 0x%x\n"
          "cr2             : 0x%X\n"
          "cr3             : 0x%X\n"
          "cr4             : 0x%x\n"
          "tpr             : 0x%x\n"
          "eflags          : 0x%x (vm:%d rf:%d iopl:%d if:%d tf:%d)\n"
          "efer            : 0x%x (lma:%d lme:%d svme:%d nxe:%d)\n"
          "gdtr (limit)    : 0x%x (0x%x)\n"
          "idtr (limit)    : 0x%x (0x%x)\n"
          ,vm_state.cpl
          ,vm_state.cr0.low
          ,vm_state.cr2.raw
          ,vm_state.cr3.raw
          ,vm_state.cr4.low
          ,vm_ctrls.int_ctrl.v_tpr
          ,vm_state.rflags.low,vm_state.rflags.vm
          ,vm_state.rflags.rf,vm_state.rflags.iopl
          ,vm_state.rflags.IF,vm_state.rflags.tf
          ,vm_state.efer.eax, vm_state.efer.lma, vm_state.efer.lme
          ,vm_state.efer.svme, vm_state.efer.nxe
          ,vm_state.gdtr.base.low, vm_state.gdtr.limit.raw
          ,vm_state.idtr.base.low, vm_state.idtr.limit.raw);
}

static void svm_vmexit_show_segment(vmcb_segment_desc_t *seg, char *name)
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

static void svm_vmexit_show_segments()
{
   svm_vmexit_show_segment(&vm_state.cs, "cs");
   svm_vmexit_show_segment(&vm_state.ss, "ss");
   svm_vmexit_show_segment(&vm_state.ds, "ds");
   svm_vmexit_show_segment(&vm_state.es, "es");
   svm_vmexit_show_segment(&vm_state.fs, "fs");
   svm_vmexit_show_segment(&vm_state.gs, "gs");
   svm_vmexit_show_segment(&vm_state.tr, "tr");
}

static void svm_vmexit_show_insn()
{
   ud_t disasm;

   if(disassemble(&disasm) == VM_DONE)
      printf("-\ninsn            : \"%s\" (len %d)\n"
             ,ud_insn_asm(&disasm),ud_insn_len(&disasm));
}

static void svm_vmexit_show_excp()
{
   uint32_t n = vm_ctrls.exit_code.low - SVM_VMEXIT_EXCP_START;

   printf("-\nexception : vector 0x%x err_code 0x%x\n",n,vm_ctrls.exit_info_1.low);
   if(n == GP_EXCP)
   {
      printf("#GP error : idx 0x%x ti %d idt %d ext %d\n"
             ,vm_ctrls.exit_info_1.excp.sl.idx
             ,vm_ctrls.exit_info_1.excp.sl.ti
             ,vm_ctrls.exit_info_1.excp.sl.ext);
   }
}

static void svm_vmexit_show_event()
{
   if(range(vm_ctrls.exit_code.low, SVM_VMEXIT_EXCP_START, SVM_VMEXIT_EXCP_END))
      svm_vmexit_show_excp();
   else if(vm_ctrls.exit_code.low == SVM_VMEXIT_INTR ||
           vm_ctrls.exit_code.low == SVM_VMEXIT_SWINT)
   {
      printf("-\ninterrupt : tpr 0x%x irq %d prio %d ign %d mask %d vector 0x%x\n"
             ,vm_ctrls.int_ctrl.v_tpr
             ,vm_ctrls.int_ctrl.v_irq
             ,vm_ctrls.int_ctrl.v_intr_prio
             ,vm_ctrls.int_ctrl.v_ign_tpr
             ,vm_ctrls.int_ctrl.v_intr_masking
             ,vm_ctrls.int_ctrl.v_intr_vector);
   }
   else if(vm_ctrls.exit_code.low == SVM_VMEXIT_SMI)
   {
      printf("-\n%s SMI raised\n"
             ,vm_ctrls.exit_info_1.smi.smi_rc?"External":"Internal");

      if(vm_ctrls.exit_info_1.smi.val)
      {
         printf("----> asserted during i/o\n"
                "i/o insn rip   : 0x%x\n"
                "acc sz         : %d byte(s)\n"
                "direction      : %s\n"
                "string insn    : %s\n"
                "rep prefix     : %s\n"
                "port           : 0x%x\n"
                "RFLAGS.TF      : %d\n"
                "brk            : 0x%x\n"
                ,vm_ctrls.exit_info_2.low
                ,(vm_ctrls.exit_info_1.high>>4) & 0x7
                ,vm_ctrls.exit_info_1.smi.d?"in":"out"
                ,vm_ctrls.exit_info_1.smi.s?"yes":"no"
                ,vm_ctrls.exit_info_1.smi.rep?"yes":"no"
                ,vm_ctrls.exit_info_1.smi.port
                ,vm_ctrls.exit_info_1.smi.tf
                ,vm_ctrls.exit_info_1.smi.brk);
      }
   }
   else if(vm_ctrls.exit_code.low == SVM_VMEXIT_IOIO)
   {
      svm_io_t *io = &vm_ctrls.exit_info_1.io;

      printf("-\n i/o  : d %d s %d rep %d port 0x%x\n"
             ,io->d,io->s,io->rep,io->port);
   }
}

static void svm_vmexit_show_deliver()
{
   if(vm_ctrls.exit_int_info.v)
   {
      char *str;

      if(vm_ctrls.exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_EXCP)
         str = exception_names[vm_ctrls.exit_int_info.vector];
      else
         str = svm_vmexit_interrupt_info_string[vm_ctrls.exit_int_info.type];

      printf("-\nidt delivery    : %s vector 0x%x err_code 0x%x\n"
             ,str,vm_ctrls.exit_int_info.vector,vm_ctrls.exit_int_info.err_code);
   }
}

static int svm_vmexit_str(char *buffer, size_t size, uint32_t err)
{
   char   *name;
   size_t sz;
   int    rc = 1;

   if(!buffer || !size)
      return 0;

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
      else
      {
         name = "Invalid VM-EXIT value";
         rc = 0;
      }

      sz = min(strlen(name), size-1);
      memcpy(buffer, name, sz);
      buffer[sz] = 0;
   }

   return rc;
}

static int svm_vmexit_show_basic()
{
   char exit_str[64];

   int rc = svm_vmexit_str(exit_str, sizeof(exit_str), vm_ctrls.exit_code.low);

   printf("\n         <------------------- VM-EXIT ------------------->\n");

   printf("-\n"
          "area start = 0x%X\n"
          "area end   = 0x%X\n"
          "area size  = %D B (%D KB)\n"
          "vmm stack  = 0x%X\n"
          "vmm pool   = 0x%X (%D KB)\n"
          "vmm elf    = 0x%X - 0x%X (%D B)\n"
          "gdt        = 0x%X\n"
          "idt        = 0x%X\n"
          "pml4       = 0x%X\n"
          "vm  vmc    = 0x%X\n"
          ,info->area.start
          ,info->area.end
          ,info->area.size, (info->area.size)>>10
          ,info->vmm.stack_bottom
          ,info->vmm.pool.addr, (info->vmm.pool.sz)>>10
          ,info->vmm.base, info->vmm.base+info->vmm.size, info->vmm.size
          ,info->vmm.cpu.sg->gdt
          ,info->vmm.cpu.sg->idt
          ,info->vmm.cpu.pg.pml4
          ,info->vm.cpu.vmc);

   printf("-\n"
          "reason          : %s (%d)\n"
          "info 1          : 0x%X\n"
          "info 2          : 0x%X\n"
          ,exit_str,vm_ctrls.exit_code.low
          ,vm_ctrls.exit_info_1.raw
          ,vm_ctrls.exit_info_2.raw);

   return rc;
}

static void svm_vmexit_show_all()
{
   svm_vmexit_show_insn();
   svm_vmexit_show_gpr();
   svm_vmexit_show_cr();
   svm_vmexit_show_segments();
   svm_vmexit_show_event();
   svm_vmexit_show_deliver();
}

void svm_vmexit_show()
{
   if(svm_vmexit_show_basic())
      svm_vmexit_show_all();
}

void svm_vmexit_failure()
{
   svm_vmexit_show();

#ifdef CONFIG_GDBSTUB
   while(1) gdbstub();
#else
   lock_vmm();
#endif
}
