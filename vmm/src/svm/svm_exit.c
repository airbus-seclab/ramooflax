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
#include <svm_exit.h>
#include <svm_exit_msr.h>
#include <svm_exit_pf.h>
#include <svm_exit_fail.h>
#include <svm_exit_excp.h>
#include <svm_exit_io.h>
#include <dev_io_ports.h>
#include <svm_vmcb.h>
#include <svm_insn.h>
#include <svm_vm.h>
#include <insn.h>
#include <intr.h>
#include <insn.h>
#include <paging.h>
#include <emulate.h>
#include <ctrl.h>
#include <db.h>
#include <info_data.h>
#include <debug.h>

/* Information data pointer set by setup */
info_data_t __info_data_hdr__ *info;

/* vm-exit handlers */
static vmexit_hdlr_t svm_vmexit_resolvers[SVM_VMEXIT_RESOLVERS_NR] = {
   resolve_default,                       //INTR
   resolve_default,                       //NMI
   resolve_default,                       //SMI
   resolve_default,                       //INIT
   resolve_default,                       //VINTR
   emulate,                               //CR0_SEL
   resolve_default,                       //IDTR_READ
   resolve_default,                       //GDTR_READ
   resolve_default,                       //LDTR_READ
   resolve_default,                       //TR_READ
   resolve_default,                       //IDTR_WRITE
   resolve_default,                       //GDTR_WRITE
   resolve_default,                       //LDTR_WRITE
   resolve_default,                       //TR_WRITE
   resolve_default,                       //RDTSC
   resolve_default,                       //RDPMC
   emulate,                               //PUSHF
   emulate,                               //POPF
   resolve_cpuid,                         //CPUID
   resolve_default,                       //RSM
   emulate,                               //IRET
   emulate,                               //SWINT
   resolve_default,                       //INVD
   resolve_default,                       //PAUSE
   resolve_default,                       //HLT
   resolve_default,                       //INVLPG
   resolve_default,                       //INVLPGA
   svm_vmexit_resolve_io,                 //IOIO
   svm_vmexit_resolve_msr,                //MSR
   resolve_default,                       //TASK
   resolve_default,                       //FERR
   resolve_default,                       //SHUTDOWN
   resolve_default,                       //VMRUN
   resolve_hypercall,                     //VMMCALL
   resolve_default,                       //VMLOAD
   resolve_default,                       //VMSAVE
   resolve_default,                       //STGI
   resolve_default,                       //CLGI
   resolve_default,                       //SKINIT
   resolve_default,                       //RDTSCP
   resolve_icebp,                         //ICEBP
   resolve_default,                       //WBINVD
   resolve_default,                       //MONITOR
   resolve_default,                       //MWAIT
   resolve_default                        //MWAIT_COND
};

static void svm_vmexit_tsc_rebase(raw64_t tsc)
{
   /* vm_ctrls.tsc_offset.sraw -= (rdtsc() - tsc.raw); */
   tsc.raw++;
}

static void svm_vmexit_pre_hdl()
{
   svm_vmsave(&info->vm.cpu.vmc->vm_vmcb);

   info->vm.cpu.gpr->rax.raw = vm_state.rax.raw;
   info->vm.cpu.gpr->rsp.raw = vm_state.rsp.raw;

   svm_reset_tlb_control();
}

static void svm_vmexit_post_hdl(raw64_t tsc)
{
   db_check_pending();

   if(controller() & (VM_FAIL|VM_FAULT))
      svm_vmexit_failure();

   vm_state.rax.raw = info->vm.cpu.gpr->rax.raw;
   vm_state.rsp.raw = info->vm.cpu.gpr->rsp.raw;
   info->vm.cpu.gpr->rax.raw = (offset_t)&info->vm.cpu.vmc->vm_vmcb;

   info->vm.cpu.emu_sts = EMU_STS_AVL;

   info->vmm.ctrl.vmexit_cnt.raw++;
   svm_vmexit_tsc_rebase(tsc);
}

void __regparm__(1) svm_vmexit_handler(raw64_t tsc)
{
   svm_vmexit_pre_hdl();

   if(svm_vmexit_resolve_dispatcher() == VM_FAIL)
      svm_vmexit_failure();

   if(!svm_vmexit_idt_deliver())
      svm_vmexit_failure();

   svm_vmexit_post_hdl(tsc);
}

int svm_vmexit_resolve_dispatcher()
{
   uint32_t err = vm_ctrls.exit_code.low;

   if(err < SVM_VMEXIT_EXCP_START || err == SVM_VMEXIT_CR0_SEL_WRITE)
      return emulate();

   if(err < SVM_VMEXIT_FIRST_RESOLVER)
      return resolve_exception();

   if(err <= SVM_VMEXIT_LAST_RESOLVER)
      return svm_vmexit_resolve(err);

   if(err == SVM_VMEXIT_NPF)
      return svm_vmexit_resolve_npf();

   return VM_FAIL;
}

int svm_vmexit_idt_deliver()
{
   if(!vm_ctrls.exit_int_info.v)
      return VM_DONE;

   if(__rmode())
      return __svm_vmexit_idt_deliver_rmode();

   return __svm_vmexit_idt_deliver_pmode();
}

int __svm_vmexit_idt_deliver_rmode()
{
   /* /\* */
   /* ** we get #GP while soft/ext interrupts */
   /* ** because of idt.limit */
   /* *\/ */
   /* if(vm_ctrls.exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_SOFT || */
   /*    vm_ctrls.exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_EXT) */
   /*    return VM_DONE; */

   /* /\* */
   /* ** cpu was delivering */
   /* ** we have nothing to inject */
   /* ** so se resume delivering */
   /* *\/ */
   /* if(!vm_ctrls.event_injection.v) */
   /* { */
   /*    if(vm_ctrls.exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_EXT) */
   /*    { */
   /* 	 debug(SVM_IDT, */
   /* 		"idt deliver IRQ (rmode): 0x%x\n", */
   /* 		vm_ctrls.exit_int_info.vector */
   /* 	   ); */
   /* 	 irq_set_pending(vm_ctrls.exit_int_info.vector - VMM_IDT_IRQ_MIN); */
   /* 	 return resolve_hard_interrupt(); */
   /*    } */
   /* } */

   debug(SVM_IDT, "idt deliver RM: unhandled scenario\n");
   return VM_FAIL;
}

int __svm_vmexit_idt_deliver_pmode()
{
   /*
   ** cpu was delivering
   ** we have nothing to inject
   ** so we resume delivering
   */
   if(!vm_ctrls.event_injection.v)
   {
      debug(SVM_IDT, "idt[0x%x:%d] eax 0x%x\n"
	    ,vm_ctrls.exit_int_info.vector
	    ,vm_ctrls.exit_int_info.type
	    ,info->vm.cpu.gpr->rax.low);

      /*
      ** XXX: next_rip not supported, emulate it
      ** only intN are soft interrupts
      ** int1, int3, into are exceptions
      */
      if(vm_ctrls.exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_SOFT)
	 vm_update_rip(2);

      vm_ctrls.event_injection.raw = vm_ctrls.exit_int_info.raw;

      if(vm_ctrls.exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_EXT &&
	 vm_ctrls.int_shadow.raw)
      {
	 debug(SVM_IDT, "interrupt shadow so inject virq\n");
	 vm_ctrls.int_ctrl.raw = 0;
	 vm_ctrls.int_ctrl.v_intr_vector = vm_ctrls.exit_int_info.vector;

	 /* if(vm_ctrls.exit_int_info.vector < 16) */
	 /*    vm_ctrls.int_ctrl.v_intr_prio = 15; */
	 /* else */
	 /*    vm_ctrls.int_ctrl.v_intr_prio = vm_ctrls.exit_int_info.vector/16; */
	 debug_warning();

	 vm_ctrls.event_injection.v = 0;
	 vm_ctrls.int_ctrl.v_irq = 1;
      }

      return VM_DONE;
   }

   debug(SVM_IDT, "already injecting: %d (%d)\n"
	 ,vm_ctrls.event_injection.vector, vm_ctrls.event_injection.type);

   /*
   ** cpu was delivering
   ** we have something to inject
   */
   /* /\* exception (to be injected) while delivering exception *\/ */
   /* if(vm_ctrls.exit_int_info.type  == VMCB_IDT_DELIVERY_TYPE_EXCP && */
   /*     vm_ctrls.event_injection.type == VMCB_IDT_DELIVERY_TYPE_EXCP) */
   /* { */
   /*    uint8_t e1 = vm_ctrls.exit_int_info.vector; */
   /*    uint8_t e2 = vm_ctrls.event_injection.vector; */

   /*    if(triple_fault(e1)) */
   /*    { */
   /* 	 debug(SVM_IDT, "triple-fault\n"); */
   /* 	 return VM_FAIL; */
   /*    } */

   /*    if(double_fault(e1, e2)) */
   /*    { */
   /* 	 debug(SVM_IDT, "double-fault: %d raised while %d\n", e2, e1); */
   /* 	 return __svm_vmexit_inject_exception(DF_EXCP, 0, 0); */
   /*    } */

   /*    /\* handled serially *\/ */
   /*    debug(SVM_IDT, "handle serially: deliver(%d:%d)/inject(%d:%d)\n" */
   /* 	     , vm_ctrls.exit_int_info.type, vm_ctrls.exit_int_info.vector */
   /* 	     , vm_ctrls.event_injection.type, vm_ctrls.event_injection.vector */
   /* 	); */

   /*    return VM_DONE; */
   /* } */

   /* debug(SVM_IDT, "un-implemented scenario: delivering(%d:%d)/injecting(%d:%d)\n" */
   /* 	  , vm_ctrls.exit_int_info.type, vm_ctrls.exit_int_info.vector */
   /* 	  , vm_ctrls.event_injection.type, vm_ctrls.event_injection.vector */
   /*   ); */

   debug(SVM_IDT, "idt deliver PM: unhandled scenario\n");
   return VM_FAIL;
}

/* { */
/* 	 int_desc_t *idt; */
/* 	 seg_desc_t *gdt; */
/* 	 tss_t      *tss; */
/* 	 offset_t   vaddr, paddr; */
/* 	 size_t     psz; */
/* 	 static int x=0; */

/* 	 if(x==0) */
/* 	 { */
/* 	    vaddr = __idtr.base.raw; */
/* 	    debug(SVM_IDT, "idt base -> 0x%X\n", vaddr); */
/* 	    pg_walk(vaddr, &paddr, &psz); */
/* 	    idt = (int_desc_t*)paddr; */

/* 	    vaddr = */
/* 	       ((idt[vm_ctrls.exit_int_info.vector].offset_2<<16) | */
/* 		idt[vm_ctrls.exit_int_info.vector].offset_1) & 0xffffffff; */
/* 	    debug(SVM_IDT, "idt vector[0x%x] -> 0x%X\n" */
/* 		  ,vm_ctrls.exit_int_info.vector, vaddr); */
/* 	    pg_walk(vaddr, &paddr, &psz); */

/* 	    vaddr = */
/* 	       ((idt[NP_EXCP].offset_2<<16) | */
/* 		idt[NP_EXCP].offset_1) & 0xffffffff; */
/* 	    debug(SVM_IDT, "idt vector[0x%x] -> 0x%X\n" */
/* 		  ,NP_EXCP, vaddr); */
/* 	    pg_walk(vaddr, &paddr, &psz); */

/* 	    vaddr = */
/* 	       ((idt[GP_EXCP].offset_2<<16) | */
/* 		idt[GP_EXCP].offset_1) & 0xffffffff; */
/* 	    debug(SVM_IDT, "idt vector[0x%x] -> 0x%X\n" */
/* 		  ,GP_EXCP, vaddr); */
/* 	    pg_walk(vaddr, &paddr, &psz); */

/* 	    vaddr = __gdtr.base.raw; */
/* 	    debug(SVM_IDT, "gdt base -> 0x%X\n", vaddr); */
/* 	    pg_walk(vaddr, &paddr, &psz); */

/* 	    gdt = (seg_desc_t*)paddr; */
/* 	    debug(SVM_IDT, "gdt[0x%x] = 0x%X (p %d)\n" */
/* 		  , (idt[vm_ctrls.exit_int_info.vector].selector)>>3 */
/* 		  , gdt[(idt[vm_ctrls.exit_int_info.vector].selector)>>3].raw */
/* 		  , gdt[(idt[vm_ctrls.exit_int_info.vector].selector)>>3].p); */

/* 	    vaddr = __tr.base.raw; */
/* 	    debug(SVM_IDT, "tr base -> 0x%X\n", vaddr); */
/* 	    pg_walk(vaddr, &paddr, &psz); */

/* 	    tss = (tss_t*)paddr; */
/* 	    debug(SVM_IDT, "tss.esp0 -> 0x%X\n", tss->s0.esp & 0xffffffff); */

/* 	    vaddr = tss->s0.esp & 0xffffffff; */
/* 	    pg_walk(vaddr, &paddr, &psz); */

/* 	    x++; */
/* 	 } */
/* } */
