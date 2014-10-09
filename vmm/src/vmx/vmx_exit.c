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
#include <vmx_exit.h>
#include <vmx_exit_cr.h>
#include <vmx_exit_dr.h>
#include <vmx_exit_dt.h>
#include <vmx_exit_msr.h>
#include <vmx_exit_pf.h>
#include <vmx_exit_fail.h>
#include <vmx_exit_excp.h>
#include <vmx_exit_io.h>
#include <dev_io_ports.h>
#include <vmx_vmcs.h>
#include <vmx_insn.h>
#include <vmx_vm.h>
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
static vmexit_hdlr_t vmx_vmexit_resolvers[VMX_VMEXIT_RESOLVERS_NR] = {
   vmx_vmexit_resolve_excp,             //EXCP_NMI
   resolve_default,                     //EXT_INT
   resolve_default,                     //TRI_FAULT
   resolve_default,                     //INIT_SIG
   resolve_default,                     //SIPI
   resolve_default,                     //IO_SMI
   resolve_default,                     //OTHER_SMI
   resolve_default,                     //INT_WIN
   resolve_default,                     //NMI_WIN
   resolve_default,                     //TASK_SW
   resolve_cpuid,                       //CPUID
   resolve_default,                     //GETSEC
   resolve_default,                     //HLT
   resolve_default,                     //INVD
   resolve_default,                     //INVLPG
   resolve_default,                     //RDPMC
   resolve_default,                     //RDTSC
   resolve_default,                     //RSM
   resolve_hypercall,                   //VMCALL
   resolve_default,                     //VMCLEAR
   resolve_default,                     //VMLAUNCH
   resolve_default,                     //VMPTRLD
   resolve_default,                     //VMPTRST
   resolve_default,                     //VMREAD
   resolve_default,                     //VMRESUME
   resolve_default,                     //VMWRITE
   resolve_default,                     //VMXOFF
   resolve_default,                     //VMXON
   vmx_vmexit_resolve_cr_access,        //CR_ACCESS
   vmx_vmexit_resolve_dr_access,        //MOV_DR
   vmx_vmexit_resolve_io,               //IO_INSN
   vmx_vmexit_resolve_msr_rd,           //RDMSR
   vmx_vmexit_resolve_msr_wr,           //WRMSR
   resolve_default,                     //INVL_G_STATE
   resolve_default,                     //MSR_LOAD
   resolve_default,                     //UNDEFINED
   resolve_default,                     //MWAIT
   resolve_default,                     //MTF
   resolve_default,                     //UNDEFINED
   resolve_default,                     //MONITOR
   resolve_default,                     //PAUSE
   resolve_default,                     //MACH_CHECK
   resolve_default,                     //UNDEFINED
   resolve_default,                     //TPR
   resolve_default,                     //APIC
   resolve_default,                     //UNDEFINED
   vmx_vmexit_resolve_dt,               //DTR
   resolve_default,                     //LDTR
   vmx_vmexit_resolve_ept_viol,         //EPT
   resolve_default,                     //EPT_CONF
   resolve_default,                     //INVEPT
   resolve_default,                     //RDTSCP
   vmx_vmexit_resolve_preempt,          //PREEMPT
   resolve_default,                     //INVVPID
   resolve_wbinvd,                      //WBINVD
   resolve_xsetbv,                     //XSETBV
};

static void vmx_vmexit_tsc_rebase(raw64_t tsc)
{
   /* vmcs_read(vm_exec_ctrls.tsc_offset); */

   /* if(!vm_exec_ctrls.tsc_offset.raw) */
   /*    vm_exec_ctrls.tsc_offset.raw = -rdtsc(); */

   /* vm_exec_ctrls.tsc_offset.sraw -= (rdtsc() - tsc.raw); */
   /* vmcs_dirty(vm_exec_ctrls.tsc_offset); */
   tsc.raw++;
}

static void vmx_vmexit_pre_hdl()
{
   vmcs_read(vm_exit_info.reason);

   vmcs_read(vm_state.rflags);
   vmcs_read(vm_state.cr0);
   vmcs_read(vm_state.cr3);
   vmcs_read(vm_exec_ctrls.cr0_read_shadow);

   vmcs_read(vm_state.cs.base);
   vmcs_read(vm_state.cs.selector);
   vmcs_read(vm_state.cs.attributes);
   vmcs_read(vm_state.rip);

   vmcs_read(vm_state.ss.base);
   vmcs_read(vm_state.rsp);

   info->vm.cpu.gpr->rsp.raw = vm_state.rsp.raw;
}

static void vmx_vmexit_post_hdl(raw64_t tsc)
{
   db_check_stp();
   controller();

   vm_state.rsp.raw = info->vm.cpu.gpr->rsp.raw;
   vmcs_dirty(vm_state.rsp);

   info->vm.cpu.emu_sts = EMU_STS_AVL;

   info->vmm.ctrl.vmexit_cnt.raw++;
   vmx_vmexit_tsc_rebase(tsc);

   vmx_vmcs_commit(info);
}

void __regparm__(1) vmx_vmexit_handler(raw64_t tsc)
{
   vmx_vmexit_pre_hdl();

   if(!vmx_vmexit_resolve_dispatcher())
      vmx_vmexit_failure();

   if(!vmx_vmexit_idt_deliver())
      vmx_vmexit_failure();

   vmx_vmexit_post_hdl(tsc);
}

int vmx_vmexit_resolve_dispatcher()
{
   return vmx_vmexit_resolve(vm_exit_info.reason.basic);
}

int vmx_vmexit_resolve_preempt()
{
   return 0;
}

int vmx_vmexit_idt_deliver()
{
   vmcs_read(vm_exit_info.idt_info);

   if(!vm_exit_info.idt_info.v)
      return 1;

   if(__rmode())
      return __vmx_vmexit_idt_deliver_rmode();

   return __vmx_vmexit_idt_deliver_pmode();
}

int __vmx_vmexit_idt_deliver_rmode()
{
   /*
   ** hard/soft int generates #GP while in rmode
   ** we discard resume as we emulate
   */
   if(vm_exit_info.idt_info.type == VMCS_IDT_INFO_TYPE_SW_INT ||
      vm_exit_info.idt_info.type == VMCS_IDT_INFO_TYPE_HW_INT)
      return 1;

   /* /\* */
   /* ** cpu was delivering */
   /* ** we have nothing to inject */
   /* ** so we resume delivering */
   /* *\/ */
   /* if(!vm_entry_ctrls.int_info.v) */
   /* { */
   /*    if(vm_exit_info.idt_info.type == VMCS_IDT_INFORMATION_TYPE_HW_INT) */
   /*    { */
   /* 	 debug(VMX_IDT, */
   /* 	       "idt deliver IRQ (rmode): 0x%x\n", */
   /* 	       vm_exit_info.idt_info.vector); */
   /* 	 irq_set_pending(vm_exit_info.idt_info.vector - IDT_IRQ_MIN); */
   /* 	 return resolve_hard_interrupt(); */
   /*    } */
   /* } */

   debug(VMX_IDT, "idt deliver RM: unhandled scenario\n");
   return 0;
}

int __vmx_vmexit_idt_deliver_pmode()
{
   vmcs_read(vm_entry_ctrls.int_info);

   /*
   ** cpu was delivering
   ** we have nothing to inject
   ** so we resume delivering
   */
   if(!vm_entry_ctrls.int_info.v)
   {
      debug(VMX_IDT, "idt[0x%x:%d] eax 0x%x\n"
	    ,vm_exit_info.idt_info.vector
	    ,vm_exit_info.idt_info.type
	    ,info->vm.cpu.gpr->rax.low);

      if(vm_exit_info.idt_info.type == VMCS_IDT_INFO_TYPE_SW_INT)
      {
	 vm_entry_ctrls.insn_len.raw = 2;
	 vmcs_dirty(vm_entry_ctrls.insn_len);
      }

      if(vm_exit_info.idt_info.v_err)
      {
	 vmcs_read(vm_exit_info.idt_err_code);
	 vm_entry_ctrls.err_code.raw = vm_exit_info.idt_err_code.raw;
	 vmcs_dirty(vm_entry_ctrls.err_code);
      }

      /*
      ** - interrupt shadow
      ** - read 31.7.1.1 reflecting exceptions to guest software
      */
      debug_warning();

      vm_entry_ctrls.int_info.raw = vm_exit_info.idt_info.raw;
      vm_entry_ctrls.int_info.r = 0; /* take care of reserved bits */
      vmcs_dirty(vm_entry_ctrls.int_info);

      return 1;
   }

   debug(VMX_IDT, "already injecting: %d (%d)\n"
	 ,vm_entry_ctrls.int_info.vector, vm_entry_ctrls.int_info.type);

   /*
   ** cpu was delivering
   ** we have something to inject
   */
/*    /\* exception (to be injected) while delivering exception *\/ */
/*    if(vm_exit_info.idt_info.type  == VMCS_IDT_INFO_TYPE_HW_EXCP && */
/*       vm_entry_ctrls.int_info.type == VMCS_IDT_INFO_TYPE_HW_EXCP) */
/*    { */
/*       uint8_t e1 = vm_exit_info.idt_info.vector; */
/*       uint8_t e2 = vm_entry_ctrls.int_info.vector; */

/*       if(triple_fault(e1)) */
/*       { */
/* 	 debug(VMX, "triple-fault\n"); */
/* 	 return 0; */
/*       } */

/*       if(double_fault(e1, e2)) */
/*       { */
/* 	 debug(VMX, "double-fault: %d raised while %d\n", e2, e1); */
/* 	 return __vmx_vmexit_inject_exception(DOUBLEFAULT_EXCEPTION, 0, 0); */
/*       } */

/*       /\* handled serially *\/ */
/*       debug(VMX_IDT, "handle serially: deliver(%d:%d)/inject(%d:%d)\n" */
/* 	    , vm_exit_info.idt_info.type, vm_exit_info.idt_info.vector */
/* 	    , vm_entry_ctrls.int_info.type, vm_entry_ctrls.int_info.vector); */
/*       return 1; */
/*    } */

/* /\* */
/*    if(vm_exit_info.idt_info.type  == VMCS_IDT_INFORMATION_TYPE_HW_INT && */
/*        vm_entry_ctrls.int_info.type == VMCS_IDT_INFORMATION_TYPE_HW_EXCP) */
/*    { */
/*    } */
/* *\/ */

   debug(VMX_IDT, "idt deliver PM: unhandled scenario\n");
   return 0;
}
