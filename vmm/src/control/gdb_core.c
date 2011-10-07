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
#include <ctrl.h>
#include <gdb.h>
#include <insn.h>
#include <excp.h>
#include <debug.h>
#include <emulate.h>
#include <info_data.h>

extern info_data_t *info;

void gdb_protect_bp_excp()
{
   info->vmm.ctrl.dbg.excp |= (1<<BP_EXCP);
   gdb_set_traps(0);
}

void gdb_release_bp_excp()
{
   info->vmm.ctrl.dbg.excp &= ~(1<<BP_EXCP);
   gdb_set_traps(0);
}

void gdb_protect_db_excp()
{
   if(gdb_singlestep_enabled())
   {
      //__deny_pushf();
      //__deny_popf();
      //__deny_iret();
      //__deny_icebp();
      //__deny_soft_int();
      //__deny_hrdw_int();
      //__deny_excp();
      //info->vmm.ctrl.dbg.excp |= (1<<GP_EXCP);
   }
   else if(!gdb_brk_hrd_enabled())
      return;

   info->vmm.ctrl.dbg.excp |= (1<<DB_EXCP);
   gdb_set_traps(0);

   debug(GDB_CMD, "installed #DB intercept (%d|%d)\n"
	 , gdb_singlestep_enabled()
	 , gdb_brk_hrd_enabled());
}

void gdb_release_db_excp()
{
   if(gdb_singlestep_enabled())
      return;

   //info->vmm.ctrl.dbg.excp &= ~(1<<GP_EXCP);
   //__allow_pushf();
   //__allow_popf();
   //__allow_iret();
   //__allow_icebp();
   //__allow_soft_int();
   //__allow_hrdw_int();
   //__allow_excp();

   if(gdb_brk_hrd_enabled())
      return;

   info->vmm.ctrl.dbg.excp &= ~(1<<DB_EXCP);
   gdb_set_traps(0);

   debug(GDB_CMD, "removed #DB intercept (%d|%d)\n"
	 , gdb_singlestep_enabled()
	 , gdb_brk_hrd_enabled());
}

/*
** sysenter, sysexit #GP
*/
static int gdb_excp_gp()
{
   ud_t disasm;
   int  rc;

   __sysenter_cs.raw = info->vmm.ctrl.dbg.stp.ctx.sysenter_cs.raw;

   rc = disassemble(&disasm);
   if(!rc)
      return GDB_FAIL;

   rc = __emulate_insn(&disasm);
   __sysenter_cs.raw = 0;
   __post_access(__sysenter_cs);

   switch(rc)
   {
   case EMU_UNSUPPORTED:
      debug(GDB_CMD, "emulation unsupported while handling #GP !\n");
      return GDB_IGNORE;
   case EMU_FAULT:
      debug(GDB_CMD, "emulation fault while handling #GP !\n");
      return GDB_FAULT;
   case EMU_FAIL:
      debug(GDB_CMD, "emulation failure while handling #GP !\n");
      return GDB_FAIL;
   }

   if(disasm.mnemonic == UD_Isysenter)
   {
      debug(GDB_CMD, "emulated sysenter\n");
      if(__gdb_active_cr3_check(0))
      {
	 if(gdb_singlestep_check())
	 {
	    debug(GDB_CMD, "removed TF\n");
	    __rflags.tf = 0;
	    __post_access(__rflags);
	 }
      }
   }
   else if(disasm.mnemonic == UD_Isysexit)
   {
      debug(GDB_CMD, "emulated sysexit\n");
      if(__gdb_active_cr3_check(0))
      {
	 debug(GDB_CMD, "simulating single-step event\n");
	 return __gdb_singlestep_fake();
      }
   }
   else
      return GDB_IGNORE;

   return GDB_RELEASE;
}

int __gdb_active_cr3_check(int rpl)
{
   if(!info->vmm.ctrl.dbg.status.cr3)
      return 1;

   if(__cpl < rpl)
      return 0;

   if(_xx_lmode())
      return (__cr3.pml4.addr == info->vmm.ctrl.dbg.stored_cr3.pml4.addr);
   else if(__cr4.pae)
      return (__cr3.pae.addr == info->vmm.ctrl.dbg.stored_cr3.pae.addr);
   else
      return (__cr3.addr == info->vmm.ctrl.dbg.stored_cr3.addr);

   return 0;
}

void gdb_traps_enable()
{
   debug(GDB_CMD, "gdb traps enable\n");

   if(gdb_singlestep_enabled())
   {
      debug(GDB_CMD, "enabled sysenter_cs #GP trick\n");
      info->vmm.ctrl.dbg.excp |= 1<<GP_EXCP;
      __sysenter_cs.raw = 0;
      __post_access(__sysenter_cs);
   }
   else if(info->vmm.ctrl.dbg.excp & (1<<GP_EXCP))
      info->vmm.ctrl.dbg.excp &= ~(1<<GP_EXCP);

   __gdb_brk_mem_rearm();
   __update_exception_mask();
   __dr7.low = info->vmm.ctrl.dbg.brk.hrd.dr7.low;
   __post_access(__dr7);
   gdb_set_traps(1);
   gdb_set_traps_updated();
}

void gdb_traps_disable()
{
   debug(GDB_CMD, "gdb traps disable\n");

   if(gdb_singlestep_enabled())
   {
      debug(GDB_CMD, "restored original sysenter_cs\n");
      info->vmm.ctrl.dbg.excp &= ~(1<<GP_EXCP);
      __sysenter_cs.raw = info->vmm.ctrl.dbg.stp.ctx.sysenter_cs.raw;
      __post_access(__sysenter_cs);
   }

   __dr7.low &= 0x100;
   __post_access(__dr7);
   __exception_bitmap.raw = info->vm.cpu.dflt_excp;
   __post_access(__exception_bitmap);

   if(!gdb_brk_mem_disarmed())
      __gdb_brk_mem_disarm();

   gdb_set_traps(0);
   gdb_set_traps_updated();
}

/*
** Hardware data, i/o, single-step traps
** are checked after insn execution. If we
** emulated one, we may loose a #db condition
**
** Such a situation is raised when we single-step
** or put data breakpoint on an instruction which
** is emulated by the vmm (cr access, ...)
**
** Insn hardware breakpoints are not subject to this
** because they are checked before execution
*/
void gdb_stub_pre()
{
   if(!__vmexit_on_insn())
      return;

   if(!gdb_active_cr3_check())
      return;

   if(gdb_singlestep_fake())
      return;

   /* XXX: we must check data and i/o breakpoints too */
}

void gdb_stub_post()
{
   if(!gdb_enabled())
      return;

   if(__gdb_active_cr3_check(0))
   {
      if(!gdb_traps_configured() || gdb_traps_need_update())
	 gdb_traps_enable();
   }
   else if(gdb_traps_configured() || gdb_traps_need_update())
      gdb_traps_disable();
}

static int gdb_excp_dbg(uint32_t vector)
{
   int     rc;
   uint8_t preempt_event = GDB_EXIT_TRAP;

   if(!(info->vmm.ctrl.dbg.excp & (1<<vector)))
      return GDB_IGNORE;

   switch(vector)
   {
   case DB_EXCP:
      rc = gdb_brk_hrd(); break;
   case BP_EXCP:
      rc = gdb_brk_mem(); break;
   case GP_EXCP:
      rc = gdb_excp_gp(); break;
   default: /* inconsistent state */
      debug(GDB_CMD, "vmm can't handle exception, we let it to remote client\n");
      preempt_event = GDB_EXIT_EXCP_DE+vector;
      rc = GDB_PREEMPT;
   }

   if(rc == GDB_PREEMPT)
      gdb_preempt(preempt_event);

   return rc;
}

static int gdb_excp_user(uint32_t vector)
{
   if(info->vmm.ctrl.usr.excp & (1<<vector))
      gdb_preempt(GDB_EXIT_EXCP_DE+vector);
  
   /* we will preempt, but force vmm to also inject it */
   return GDB_IGNORE;
}

int gdb_cr_rd_event(uint8_t n)
{
   if(info->vmm.ctrl.usr.cr_rd & (1<<n))
      gdb_preempt(GDB_EXIT_R_CR0+n);

   return 1;
}

int gdb_cr_wr_event(uint8_t n)
{
   if(info->vmm.ctrl.usr.cr_wr & (1<<n))
      gdb_preempt(GDB_EXIT_W_CR0+n);

   return 1;
}

int gdb_excp_event(uint32_t vector)
{
   int rc;

   debug(GDB_CMD, "GDB Exception Event #%d\n", vector);

   rc = gdb_excp_dbg(vector);

   if(rc == GDB_IGNORE)
      rc = gdb_excp_user(vector);

   return rc;
}
