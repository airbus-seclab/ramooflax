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
#include <int.h>
#include <pf.h>
#include <info_data.h>
#include <debug.h>

/* Information data pointer set by loader */
info_data_t __info_data_hdr__ *info;

/* vm-exit handlers */
static vmexit_hdlr_t vmx_vmexit_resolvers[] = {
   vmx_vmexit_resolve_exception,        //EXCP_NMI
   resolve_preempt_hard_interrupt,      //EXT_INT
   resolve_default,                     //TRI_FAULT
   resolve_default,                     //INIT_SIG
   resolve_default,                     //SIPI
   resolve_default,                     //IO_SMI
   resolve_default,                     //OTHER_SMI
   resolve_virt_interrupt,              //INT_WIN
   resolve_default,                     //NMI_WIN
   resolve_default,                     //TASK_SW
   resolve_cpuid,                       //CPUID
   resolve_default,                     //UNDEFINED
   resolve_hlt,                         //HLT
   resolve_invd,                        //INVD
   vmx_vmexit_resolve_invlpg,           //INVLPG
   resolve_default,                     //RDPMC
   resolve_default,                     //RDTSC
   resolve_default,                     //RSM
   vmx_vmexit_resolve_vmcall,           //VMCALL
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
   resolve_default,                     //MOV_DR
   vmx_vmexit_resolve_io,               //IO_INSN
   vmx_vmexit_resolve_msr_rd,           //RDMSR
   vmx_vmexit_resolve_msr_wr,           //WRMSR
   resolve_default,                     //INVL_G_STATE
   resolve_default,                     //MSR_LOAD
   resolve_default,                     //UNDEFINED
   resolve_default,                     //MWAIT
   resolve_default,                     //UNDEFINED
   resolve_default,                     //UNDEFINED
   resolve_default,                     //MONITOR
   resolve_default,                     //PAUSE
   resolve_default,                     //MACH_CHECK
   resolve_default,                     //UNDEFINED
   resolve_default                      //TPR
};

/*
** VM-exit high level handler
*/
void __attribute__ ((regparm(2))) vmx_vmexit_handler(uint32_t rdtsc_l, uint32_t rdtsc_h)
{
   raw64_t         start;
   vmcs_exec_ctl_t *ctls = exec_ctrls(info);

   info->vmm.ctrl.vmexit_cnt.raw++;

   /*
   ** XXX: synchronise VMCS.RSP with GPR.RSP !!!!
   */

   if( ! vmx_vmexit_resolve_dispatcher() )
      vmx_vmexit_failure();

   if( ! vmx_vmexit_idt_deliver() )
      vmx_vmexit_failure();

   vmx_vmcs_commit( info );

   /* XXX incorrect, time elapsed after last rdtsc() */
   start.low  = rdtsc_l;
   start.high = rdtsc_h;

   vmcs_read( ctls->tsc_offset );

   if( ! ctls->tsc_offset.raw )
      ctls->tsc_offset.raw = -rdtsc();

   ctls->tsc_offset.sraw -= (rdtsc() - start.raw);

   vmcs_force_flush( ctls->tsc_offset );
}

int vmx_vmexit_resolve_dispatcher()
{
   vmcs_exit_info_t *exit    = exit_info(info);
   vmcs_exec_ctl_t  *exec    = exec_ctrls(info);
   vmcs_guest_t     *g_state = guest_state(info);

   vmcs_read( exit->reason );

   vmcs_read( exec->cr0_read_shadow );
   vmcs_read( exec->cr4_read_shadow );

   vmcs_read( g_state->cr0 );
   vmcs_read( g_state->rip );
   vmcs_read( g_state->cs.base_addr );
   vmcs_read( g_state->cs.selector );
   vmcs_read( g_state->cs.attributes );
   vmcs_read( g_state->rflags );

   return vmx_vmexit_resolve( exit->reason.basic );
}

int vmx_vmexit_idt_deliver()
{
   vmcs_exit_info_t *exit  = exit_info(info);

   vmcs_read( exit->idt_info );

   /* cpu was not delivering */
   if( ! exit->idt_info.v )
      return 1;

   if( __rmode() )
      return __vmx_vmexit_idt_deliver_rmode();

   return __vmx_vmexit_idt_deliver_pmode();
}

int __vmx_vmexit_idt_deliver_rmode()
{
   vmcs_exit_info_t *exit  = exit_info(info);
   vmcs_entry_ctl_t *entry = entry_ctrls(info);

   /*
   ** soft int generates #GP while in rmode
   ** we discard resume as we emulate
   */
   if( exit->idt_info.type == VMCS_IDT_INFORMATION_TYPE_SW_INT )
      return 1;

   /*
   ** cpu was delivering
   ** we have nothing to inject
   ** so we resume delivering
   */
   if( ! entry->int_info.v )
   {
      if( exit->idt_info.type == VMCS_IDT_INFORMATION_TYPE_HW_INT )
      {
	 debug( VMX_IDT,
		"idt deliver IRQ (rmode): 0x%x\n",
		exit->idt_info.vector
	    );
	 irq_set_pending( exit->idt_info.vector - IDT_IRQ_MIN );
	 return resolve_hard_interrupt();
      }
   }

   debug( VMX_IDT, "idt deliver RM: unhandled scenario\n" );
   return 0;
}

int __vmx_vmexit_idt_deliver_pmode()
{
   vmcs_exit_info_t *exit  = exit_info(info);
   vmcs_entry_ctl_t *entry = entry_ctrls(info);

   /*
   ** cpu was delivering
   ** we have nothing to inject
   ** so we resume delivering
   */
   if( ! entry->int_info.v )
   {
      if( exit->idt_info.type == VMCS_IDT_INFORMATION_TYPE_HW_EXCP &&
	  exit->idt_info.vector == PAGEFAULT_EXCEPTION )
      {
	 vmcs_read( exit->idt_err_code );

	 debug( VMX_IDT, "idt deliver check #PF (0x%x, 0x%x)\n",
		get_cr2(), exit->idt_err_code.raw );

	 return resolve_pf( get_cr2(), exit->idt_err_code.raw );
      }

      if( exit->idt_info.type == VMCS_IDT_INFORMATION_TYPE_HW_INT )
      {
	 debug( VMX_IDT,
		"idt deliver IRQ (pmode): 0x%x\n",
		exit->idt_info.vector
	    );
	 irq_set_pending( exit->idt_info.vector - IDT_IRQ_MIN );
	 return resolve_hard_interrupt();
      }

      if( exit->idt_info.v_err )
      {
	 vmcs_read( exit->idt_err_code );
	 entry->err_code.raw = exit->idt_err_code.raw;
	 vmcs_dirty( entry->err_code );
      }

      debug( VMX_IDT, "idt deliver pending (%d,0x%x) eax 0x%x !\n"
	     , exit->idt_info.type
	     , exit->idt_info.vector
	     , info->vm.cpu.gpr->eax.raw
	 );

      entry->int_info.raw = exit->idt_info.raw;
      vmcs_dirty( entry->int_info );
      return 1;
   }

   /*
   ** cpu was delivering
   ** we have something to inject
   */

   /* exception (to be injected) while delivering exception */
   if( exit->idt_info.type  == VMCS_IDT_INFORMATION_TYPE_HW_EXCP &&
       entry->int_info.type == VMCS_IDT_INFORMATION_TYPE_HW_EXCP )
   {
      uint8_t e1 = exit->idt_info.vector;
      uint8_t e2 = entry->int_info.vector;

      if( triple_fault(e1) )
      {
	 debug( VMX, "triple-fault\n" );
	 return 0;
      }

      if( double_fault(e1, e2) )
      {
	 debug( VMX, "double-fault: %d raised while %d\n", e2, e1 );
	 return __vmx_vmexit_inject_exception( DOUBLEFAULT_EXCEPTION, 0, 0 );
      }

      /* handled serially */
      debug( VMX_IDT, "handle serially: deliver(%d:%d)/inject(%d:%d)\n"
	     , exit->idt_info.type, exit->idt_info.vector
	     , entry->int_info.type, entry->int_info.vector
	 );

      return 1;
   }

/*
   if( exit->idt_info.type  == VMCS_IDT_INFORMATION_TYPE_HW_INT &&
       entry->int_info.type == VMCS_IDT_INFORMATION_TYPE_HW_EXCP )
   {
   }
*/

   debug( VMX_IDT, "un-implemented scenario: delivering(%d:%d)/injecting(%d:%d)\n"
	  , exit->idt_info.type, exit->idt_info.vector
	  , entry->int_info.type, entry->int_info.vector
      );

   return 0;
}

void vmx_release_irq()
{
   vmcs_exec_ctl_t *ctls = exec_ctrls(info);

   vmcs_read( ctls->proc );
   vmcs_read( ctls->pin );

   vmx_allow_io_range( info->vm.cpu.vmc, PIC1_START_PORT, PIC1_END_PORT );
   vmx_allow_io_range( info->vm.cpu.vmc, PIC2_START_PORT, PIC2_END_PORT );

   ctls->proc.hlt  = 0;
   ctls->pin.eint  = 0;
   ctls->proc.iwe  = 0;

   vmcs_dirty( ctls->proc );
   vmcs_dirty( ctls->pin );
}

int vmx_vmexit_resolve_vmcall()
{
   vmcs_exit_info_t *exit  = exit_info(info);
   vmcs_guest_t     *state = guest_state(info);

   vmcs_read( exit->vmexit_insn_len );

   debug( VMX, "-- vmcall --\n" );

   state->rip.low += exit->vmexit_insn_len.raw;
   vmcs_dirty( state->rip );

   return 1;
}
