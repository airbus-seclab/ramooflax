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
#include <vmx_vmcs_acc.h>
#include <emulate.h>
#include <vmm.h>
#include <info_data.h>
#include <debug.h>
#include <excp.h>
#include <klibc.h>

extern info_data_t *info;

char vmx_vmresume_failure_fmt[] = "failed (%d)\n";
char vmx_vmresume_failure_fnm[] = "vmresume";

extern char* exception_names[];

static char* vmx_vmexit_basic_reason_names[] = {
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
   "-- Undefined --",
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
   "MOV DR",
   "I/O Insn",
   "RDMSR",
   "WRMSR",
   "Invalid Guest State (VM-entry)",
   "MSR Loading (VM-entry)",
   "-- Undefined --",
   "MWAIT",
   "-- Undefined --",
   "-- Undefined --",
   "MONITOR",
   "PAUSE",
   "Machine-Check (VM-entry)",
   "-- Undefined --",
   "TPR threshold"
};

void vmx_vmexit_failure()
{
   __vmx_vmexit_failure();
   panic( "failed to resolve vm-exit !\n" );
}

void __vmx_vmexit_failure()
{
   if( vmx_vmexit_collect() )
      vmx_vmexit_show();
}

int vmx_vmexit_collect()
{
   vmcs_exit_info_t *exit = exit_info(info);

   if( exit->reason.basic > VMCS_VM_EXIT_INFORMATION_BASIC_REASON_MAX )
   {
      debug( VMX, "basic reason (%d) out of range\n", exit->reason.basic );
      return 0;
   }

   vmx_vmcs_collect( info );
   return 1;
}

void vmx_vmexit_dump_code()
{
   uint32_t i;
   loc_t    loc;

   if( __paging() )
      return;

   if( __rmode() )
      loc.linear = __cs.base_addr.low + __rip.wlow;
   else
      loc.linear = __cs.base_addr.low + __rip.low;

   debug( VMX, "mem dump code @ 0x%x : ", loc.linear );
   for( i=0 ; i<15 ; i++ )
      debug( VMX, "0x%x ", *loc.u8++ );
   debug( VMX, "\n");
}

void vmx_vmexit_dump_stack()
{
   uint32_t i, j;
   loc_t    loc;

   if( __paging() )
      return;

   if( __rmode() )
   {
      loc.linear = __ss.base_addr.low + __rsp.wlow;
      debug( VMX, "mem dump stack @ 0x%x:\n", loc.linear );
      for( i=0 ; i<6 ; i++ )
      {
	 debug( VMX, "0x%x:", loc.u16 );
	 for( j=0; j<4; j++ )
	    debug( VMX, " 0x%x", *loc.u16++ );
	 debug( VMX, "\n");
      }
   }
   else
   {
      loc.linear = __ss.base_addr.low + __rsp.low;
      debug( VMX, "mem dump stack @ 0x%x:\n", loc.linear );
      for( i=0 ; i<6 ; i++ )
      {
	 debug( VMX, "0x%x:", loc.u32 );
	 for( j=0; j<4 ; j++)
	    debug( VMX, " 0x%x", *loc.u32++ );
	 debug( VMX, "\n" );
      }
   }
}

void vmx_vmexit_show()
{
   uint16_t         basic_reason;
   vmcs_guest_t     *state;
   vmcs_exec_ctl_t  *ctls;
   vmcs_exit_info_t *exit;
   ud_t             disasm;

   state = guest_state(info);
   ctls  = exec_ctrls(info);
   exit  = exit_info(info);

   debug( VMX, "\n         <------------------- VM-EXIT ------------------->\n" );

   if( emulate_disasm( &disasm ) )
      debug( VMX, "insn         : (%d) \"%s\"\n", ud_insn_len(&disasm), ud_insn_asm(&disasm) );

   vmx_vmexit_dump_code();
   vmx_vmexit_dump_stack();

   debug( VMX,
	  "-\neax = 0x%x ebx = 0x%x\n"
	  "ecx = 0x%x edx = 0x%x\n"
	  "ebp = 0x%x esi = 0x%x\n"
	  "edi = 0x%x\n"
	  ,info->vm.cpu.gpr->eax
	  ,info->vm.cpu.gpr->ebx
	  ,info->vm.cpu.gpr->ecx
	  ,info->vm.cpu.gpr->edx
	  ,info->vm.cpu.gpr->ebp
	  ,info->vm.cpu.gpr->esi
	  ,info->vm.cpu.gpr->edi );

   basic_reason = (uint16_t)exit->reason.basic;

   debug( VMX, "-\ncr0          : real 0x%x shadow 0x%x\n", state->cr0.low, ctls->cr0_read_shadow.low );
   debug( VMX, "cr3          : real 0x%x shadow 0x%x\n", state->cr3.low, info->vm.cr3_shadow );
   debug( VMX, "cr4          : real 0x%x shadow 0x%x\n", state->cr4.low, ctls->cr4_read_shadow.low );
   debug( VMX, "dr7          : 0x%x\n", state->dr7.raw );
   debug( VMX, "rip          : 0x%x\n", state->rip.low );
   debug( VMX, "rsp          : 0x%x\n", state->rsp.low );
   debug( VMX, "rflags       : 0x%x (id:%d vip:%d vif:%d vm:%d rf:%d nt:%d iopl:%d if:%d tf:%d)\n"
	  ,state->rflags.low
	  ,state->rflags.id
	  ,state->rflags.vip
	  ,state->rflags.vif
	  ,state->rflags.vm
	  ,state->rflags.rf
	  ,state->rflags.nt
	  ,state->rflags.iopl
	  ,state->rflags.IF
	  ,state->rflags.tf );

   debug( VMX, "gdtr (limit) : 0x%x (%d)\n", state->gdtr.base_addr.low, state->gdtr.limit );
   debug( VMX, "idtr (limit) : 0x%x (%d)\n", state->idtr.base_addr.low, state->idtr.limit );
   debug( VMX, "cpl          : %d\n", __cpl );

   debug( VMX, "-\ncs.base_addr : 0x%x\n", state->cs.base_addr.low );
   debug( VMX, "cs.selector  : 0x%x (idx:%d rpl:%d ti:%d)\n",
	  state->cs.selector.raw,
	  state->cs.selector.index, state->cs.selector.rpl, state->cs.selector.ti );
   debug( VMX, "cs.limit     : 0x%x\n", state->cs.limit.raw );
   debug( VMX, "cs.access    : dpl:0x%x type:0x%x db:%d g:%d p:%d\n",
	  state->cs.attributes.dpl, state->cs.attributes.type,
	  state->cs.attributes.db, state->cs.attributes.gran,
	  state->cs.attributes.p );

   debug( VMX, "-\nds.base_addr : 0x%x\n", state->ds.base_addr.low );
   debug( VMX, "ds.selector  : 0x%x (idx:%d rpl:%d ti:%d)\n",
	  state->ds.selector.raw,
	  state->ds.selector.index, state->ds.selector.rpl, state->ds.selector.ti );
   debug( VMX, "ds.limit     : 0x%x\n", state->ds.limit.raw );
   debug( VMX, "ds.access    : dpl:0x%x type:0x%x db:%d g:%d p:%d\n",
	  state->ds.attributes.dpl, state->ds.attributes.type,
	  state->ds.attributes.db, state->ds.attributes.gran,
	  state->ds.attributes.p );

   debug( VMX, "-\nss.base_addr : 0x%x\n", state->ss.base_addr.low );
   debug( VMX, "ss.selector  : 0x%x (idx:%d rpl:%d ti:%d)\n", state->ss.selector.raw, state->ss.selector.index, state->ss.selector.rpl, state->ss.selector.ti );
   debug( VMX, "ss.limit     : 0x%x\n", state->ss.limit.raw );
   debug( VMX, "ss.access    : dpl:0x%x type:0x%x db:%d g:%d p:%d\n",
	  state->ss.attributes.dpl, state->ss.attributes.type,
	  state->ss.attributes.db, state->ss.attributes.gran,
	  state->ss.attributes.p );

   debug( VMX, "-\nvm-exit            : %s (%d)\n", vmx_vmexit_basic_reason_names[ basic_reason ], basic_reason );
   debug( VMX, "due to vm-entry    : %s\n", exit->reason.entry ? "yes":"no" );
   debug( VMX, "from vmx root      : %s\n", exit->reason.root ? "yes":"no" );

   debug( VMX, "-\n" );
   switch( basic_reason )
   {
   case VMCS_VM_EXIT_INFORMATION_BASIC_REASON_CR_ACCESS:
      debug( VMX, "cr num          : %d\n", exit->qualification.cr_access.nr );
      debug( VMX, "type            : %d\n", exit->qualification.cr_access.type );
      debug( VMX, "lmsw op         : %d\n", exit->qualification.cr_access.lmsw_op );
      debug( VMX, "gpr             : %d\n", exit->qualification.cr_access.gpr );
      debug( VMX, "lmsw data       : 0x%x\n", exit->qualification.cr_access.lmsw_data );
      break;
   case VMCS_VM_EXIT_INFORMATION_BASIC_REASON_IO_INSN:
      debug( VMX, "   acc sz       : %d byte(s)\n", exit->qualification.io_insn.sz+1 );
      debug( VMX, "   direction    : %s\n", exit->qualification.io_insn.d?"in":"out" );
      debug( VMX, "   string insn  : %s\n", exit->qualification.io_insn.s?"yes":"no" );
      debug( VMX, "   rep prefix   : %s\n", exit->qualification.io_insn.rep?"yes":"no" );
      debug( VMX, "   operand      : %s\n", exit->qualification.io_insn.op?"imm":"dx" );
      debug( VMX, "   port         : 0x%x\n", exit->qualification.io_insn.port );
      break;
   case VMCS_VM_EXIT_INFORMATION_BASIC_REASON_INVL_G_STATE:
   {
      vmcs_entry_ctl_t  *entry_ctl = entry_ctrls(info);

      switch( exit->qualification.low )
      {
      case 2:
	 debug( VMX, "page directory register related load failure\n" );
	 break;
      case 3:
	 debug( VMX, "NMI injection failure\n" );
	 break;
      case 4:
	 debug( VMX, "invalid VMCS link pointer\n" );
	 break;
      default:
	 debug( VMX, "(unused) %d\n", exit->qualification.low );
	 break;
      }

      debug( VMX, "vm-entry int valid (%d)\n", entry_ctl->int_info.v );
      debug( VMX, "vm-entry ia32e (%d)\n", entry_ctl->entry.ia32e );
      debug( VMX, "vm-entry smm (%d)\n", entry_ctl->entry.smm );
      debug( VMX, "vm-entry dual (%d)\n", entry_ctl->entry.dual );
      debug( VMX,
	     "tr.ti %d\n"
	     "ldtr.ti %d\n"
	     "ss.rpl %d == cs.rpl %d\n"
	     "tr.base 0x%x\n"
	     "fs.base 0x%x\n"
	     "gs.base 0x%x\n"
	     "ldtr.base 0x%x\n"
	     , state->tr.selector.ti
	     , state->ldtr.selector.ti
	     , state->ss.selector.rpl
	     , state->cs.selector.rpl
	     , state->tr.base_addr.low
	     , state->fs.base_addr.low
	     , state->gs.base_addr.low
	     , state->ldtr.base_addr.low
	 );
      debug( VMX,
	     "cs.type %d ss.type %d ds.type %d es.type %d fs.type %d gs.type %d\n"
	     "cs.s %d ss.s %d ds.s %d es.s %d fs.s %d gs.s %d\n"
	     "cs.dpl %d ss.dpl %d ds.dpl %d es.dpl %d fs.dpl %d gs.dpl %d\n"
	     "cs.p %d ss.p %d ds.p %d es.p %d fs.p %d gs.p %d\n"
	     , state->cs.attributes.type, state->ss.attributes.type,state->ds.attributes.type,state->es.attributes.type,state->fs.attributes.type,state->gs.attributes.type
	     , state->cs.attributes.s, state->ss.attributes.s,state->ds.attributes.s,state->es.attributes.s,state->fs.attributes.s,state->gs.attributes.s
	     , state->cs.attributes.dpl, state->ss.attributes.dpl,state->ds.attributes.dpl,state->es.attributes.dpl,state->fs.attributes.dpl,state->gs.attributes.dpl
	     , state->cs.attributes.p, state->ss.attributes.p,state->ds.attributes.p,state->es.attributes.p,state->fs.attributes.p,state->gs.attributes.p
	 );
      debug( VMX,
	     "cs.r1 %d cs.r2 %d cs.r3 %d\n"
	     "ss.r1 %d ss.r2 %d ss.r3 %d\n"
	     "ds.r1 %d ds.r2 %d ds.r3 %d\n"
	     "es.r1 %d es.r2 %d es.r3 %d\n"
	     "fs.r1 %d fs.r2 %d fs.r3 %d\n"
	     "gs.r1 %d gs.r2 %d gs.r3 %d\n"
	     , state->cs.attributes.r1, state->cs.attributes.r2, state->cs.attributes.r3
	     , state->ss.attributes.r1, state->ss.attributes.r2, state->ss.attributes.r3
	     , state->ds.attributes.r1, state->ds.attributes.r2, state->ds.attributes.r3
	     , state->es.attributes.r1, state->es.attributes.r2, state->es.attributes.r3
	     , state->fs.attributes.r1, state->fs.attributes.r2, state->fs.attributes.r3
	     , state->gs.attributes.r1, state->gs.attributes.r2, state->gs.attributes.r3
	 );
      debug( VMX,
	     "cs.db %d\n"
	     "tr.type %d tr.g %d tr.s %d tr.p %d tr.r1 %d tr.r2 %d tr.r3 %d tr.unuse %d tr.limit 0x%x\n"
	     "ldtr.type %d ldtr.s %d ldtr.p %d ldtr.r1 %d ldtr.r2 %d ldtr.r3 %d ldtr.unuse %d ldtr.limit 0x%x\n"
	     , state->cs.attributes.db
	     , state->tr.attributes.type, state->tr.attributes.gran, state->tr.attributes.s, state->tr.attributes.p, state->tr.attributes.r1, state->tr.attributes.r2, state->tr.attributes.r3, state->tr.attributes.unuse, state->tr.limit.raw
	     , state->ldtr.attributes.type , state->ldtr.attributes.s, state->ldtr.attributes.p, state->ldtr.attributes.r1, state->ldtr.attributes.r2, state->ldtr.attributes.r3, state->ldtr.attributes.unuse, state->ldtr.limit.raw
	 );
      debug( VMX,
	     "activity state 0x%x\n"
	     "interruptibility state sti %d mss %d smi %d nmi %d r %d\n"
	     "debug excp r1 %d r2 %d r3 %d bs %d\n"
	     "vmcs link 0x%x\n"
	     , state->activity_state.raw
	     , state->int_state.sti, state->int_state.mss, state->int_state.smi, state->int_state.nmi, state->int_state.r
	     , state->dbg_excp.r1, state->dbg_excp.r2, state->dbg_excp.r3, state->dbg_excp.bs
	     , state->vmcs_link_ptr.low
	 );

      break;
   }
   default:
      break;
   }

   if( exit->int_info.v )
      debug( VMX,
	     "-\nvector %s (%d:0x%x)\n"
	     , vmx_vmexit_string_from_vector_type(exit->int_info.type,exit->int_info.vector)
	     , exit->int_info.vector
	     , exit->int_info.v_err?exit->int_err_code.raw:0
	 );

   if( exit->idt_info.v )
      debug( VMX,
	     "-\nidt delivery %s (%d:0x%x)\n"
	     , vmx_vmexit_string_from_vector_type(exit->idt_info.type,exit->idt_info.vector)
	     , exit->idt_info.vector
	     , exit->idt_info.v_err?exit->idt_err_code.raw:0
	 );
}

char* vmx_vmexit_string_from_vector_type(uint8_t type,  uint8_t vector)
{
   switch( type )
   {
   case VMCS_INT_INFORMATION_TYPE_HW_EXCP:
      return exception_names[vector];
   case VMCS_INT_INFORMATION_TYPE_HW_INT:
      return "IRQ";
   }

   return "(unknown)";
}
