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
#include <db.h>
#include <ctrl.h>
#include <info_data.h>
#include <debug.h>

/* Information data pointer set by setup */
info_data_t __info_data_hdr__ *info;

/* vm-exit handlers */
static vmexit_hdlr_t svm_vmexit_resolvers[SVM_VMEXIT_RESOLVERS_NR] = {
   resolve_intr,                          //INTR
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
   resolve_default,                       //VMMCALL
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
   /* vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area; */
   /* ctrls->tsc_offset.sraw -= (rdtsc() - tsc.raw); */

   tsc.raw++;
}

static void svm_vmexit_pre_hdl()
{
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;
   vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area;

   svm_vmsave(&info->vm.cpu.vmc->vm_vmcb);
   info->vm.cpu.gpr->rax.raw = state->rax.raw;
   info->vm.cpu.gpr->rsp.raw = state->rsp.raw;

   if(ctrls->tlb_ctrl.tlb_control != VMCB_TLB_CTL_NONE)
      ctrls->tlb_ctrl.tlb_control = VMCB_TLB_CTL_NONE;
}

static void svm_vmexit_post_hdl(raw64_t tsc)
{
   vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area;

   vmm_ctrl();
   db_post_hdl();

   state->rax.raw = info->vm.cpu.gpr->rax.raw;
   state->rsp.raw = info->vm.cpu.gpr->rsp.raw;
   info->vm.cpu.gpr->rax.raw = (offset_t)&info->vm.cpu.vmc->vm_vmcb;

   info->vmm.ctrl.vmexit_cnt.raw++;
   svm_vmexit_tsc_rebase(tsc);
}

void __attribute__ ((regparm(1))) svm_vmexit_handler(raw64_t tsc)
{
   svm_vmexit_pre_hdl();

   if(!svm_vmexit_resolve_dispatcher())
      svm_vmexit_failure();

   if(!svm_vmexit_idt_deliver())
      svm_vmexit_failure();

   svm_vmexit_post_hdl(tsc);
}

int svm_vmexit_resolve_dispatcher()
{
   uint32_t err = info->vm.cpu.vmc->vm_vmcb.ctrls_area.exit_code.low;

   if(err < SVM_VMEXIT_EXCP_START || err == SVM_VMEXIT_CR0_SEL_WRITE)
      return emulate();

   if(err < SVM_VMEXIT_FIRST_RESOLVER)
      return svm_vmexit_resolve_exception();

   if(err <= SVM_VMEXIT_LAST_RESOLVER)
      return svm_vmexit_resolve(err);

   if(err == SVM_VMEXIT_NPF)
      return svm_vmexit_resolve_npf();

   return 0;
}

int svm_vmexit_idt_deliver()
{
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;

   if(!ctrls->exit_int_info.v)
      return 1;

   if(__rmode())
      return __svm_vmexit_idt_deliver_rmode();

   return __svm_vmexit_idt_deliver_pmode();
}

int __svm_vmexit_idt_deliver_rmode()
{
   /* vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area; */

   /* /\* */
   /* ** we get #GP while soft/ext interrupts */
   /* ** because of idt.limit */
   /* *\/ */
   /* if(ctrls->exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_SOFT || */
   /*    ctrls->exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_EXT) */
   /*    return 1; */

   /* /\* */
   /* ** cpu was delivering */
   /* ** we have nothing to inject */
   /* ** so se resume delivering */
   /* *\/ */
   /* if(!ctrls->event_injection.v) */
   /* { */
   /*    if(ctrls->exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_EXT) */
   /*    { */
   /* 	 debug(SVM_IDT, */
   /* 		"idt deliver IRQ (rmode): 0x%x\n", */
   /* 		ctrls->exit_int_info.vector */
   /* 	   ); */
   /* 	 irq_set_pending(ctrls->exit_int_info.vector - VMM_IDT_IRQ_MIN); */
   /* 	 return resolve_hard_interrupt(); */
   /*    } */
   /* } */

   debug(SVM_IDT, "idt deliver RM: unhandled scenario\n");
   return 0;
}

int __svm_vmexit_idt_deliver_pmode()
{
   /* vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area; */

   /* /\* */
   /* ** cpu was delivering */
   /* ** we have nothing to inject */
   /* ** so we resume delivering */
   /* *\/ */
   /* if(!ctrls->event_injection.v) */
   /* { */
   /*    vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area; */

   /*    if(ctrls->exit_int_info.type   == VMCB_IDT_DELIVERY_TYPE_EXCP && */
   /* 	  ctrls->exit_int_info.vector == PG_EXCP) */
   /*    { */
   /* 	 pf_err_t err; */
   /* 	 debug(SVM_IDT, "idt deliver check #PF (0x%x, 0x%x)\n", */
   /* 		state->cr2.low, ctrls->exit_int_info.err_code); */

   /* 	 err.raw = ctrls->exit_int_info.err_code; */
   /* 	 return resolve_pf(state->cr2.low, err); */
   /*    } */

   /*    if(ctrls->exit_int_info.type == VMCB_IDT_DELIVERY_TYPE_EXT) */
   /*    { */
   /* 	 debug(SVM_IDT, */
   /* 		"idt deliver IRQ (pmode): 0x%x\n", */
   /* 		ctrls->exit_int_info.vector */
   /* 	   ); */
   /* 	 irq_set_pending(ctrls->exit_int_info.vector - VMM_IDT_IRQ_MIN); */
   /* 	 return resolve_hard_interrupt(); */
   /*    } */

   /*    debug(SVM_IDT, "idt deliver pending (%d,0x%x) eax 0x%x !\n" */
   /* 	     , ctrls->exit_int_info.type */
   /* 	     , ctrls->exit_int_info.vector */
   /* 	     , info->vm.cpu.gpr->eax.raw */
   /* 	); */

   /*    ctrls->event_injection.raw = ctrls->exit_int_info.raw; */
   /*    return 1; */
   /* } */

   /* /\* */
   /* ** cpu was delivering */
   /* ** we have something to inject */
   /* *\/ */

   /* /\* exception (to be injected) while delivering exception *\/ */
   /* if(ctrls->exit_int_info.type   == VMCB_IDT_DELIVERY_TYPE_EXCP && */
   /*     ctrls->event_injection.type == VMCB_IDT_DELIVERY_TYPE_EXCP) */
   /* { */
   /*    uint8_t e1 = ctrls->exit_int_info.vector; */
   /*    uint8_t e2 = ctrls->event_injection.vector; */

   /*    if(triple_fault(e1)) */
   /*    { */
   /* 	 debug(SVM_IDT, "triple-fault\n"); */
   /* 	 return 0; */
   /*    } */

   /*    if(double_fault(e1, e2)) */
   /*    { */
   /* 	 debug(SVM_IDT, "double-fault: %d raised while %d\n", e2, e1); */
   /* 	 return __svm_vmexit_inject_exception(DF_EXCP, 0, 0); */
   /*    } */

   /*    /\* handled serially *\/ */
   /*    debug(SVM_IDT, "handle serially: deliver(%d:%d)/inject(%d:%d)\n" */
   /* 	     , ctrls->exit_int_info.type, ctrls->exit_int_info.vector */
   /* 	     , ctrls->event_injection.type, ctrls->event_injection.vector */
   /* 	); */

   /*    return 1; */
   /* } */

   /* debug(SVM_IDT, "un-implemented scenario: delivering(%d:%d)/injecting(%d:%d)\n" */
   /* 	  , ctrls->exit_int_info.type, ctrls->exit_int_info.vector */
   /* 	  , ctrls->event_injection.type, ctrls->event_injection.vector */
   /*   ); */

   debug(SVM_IDT, "idt deliver PM: unhandled scenario\n");
   return 0;
}

