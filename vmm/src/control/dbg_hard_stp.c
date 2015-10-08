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
#include <dbg_hard_stp.h>
#include <disasm.h>
#include <emulate.h>
#include <emulate_insn.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void dbg_hard_stp_save_context()
{
   __pre_access(__sysenter_cs);
   __hstp_ctx.sysenter_cs.raw = __sysenter_cs.raw;
}

void dbg_hard_stp_setup_context()
{
   debug(DBG_HARD_STP, "sstep install\n");
   __sysenter_cs.raw = 0;
   __post_access(__sysenter_cs);
}

void dbg_hard_stp_restore_context()
{
   debug(DBG_HARD_STP, "sstep restore\n");
   __sysenter_cs.raw = __hstp_ctx.sysenter_cs.raw;
   __post_access(__sysenter_cs);
}

static void dbg_hard_stp_protect()
{
   if(!dbg_hard_stp_enabled())
      return;

   debug(DBG_HARD_STP, "sstep protect\n");
   dbg_hard_stp_save_context();

   dbg_hard_stp_save_tf(__rflags.tf);
   __rflags.tf = 1;
   __post_access(__rflags);

   /* XXX */
   /* __protect_rflags(); */

   info->vmm.ctrl.dbg.excp |= 1<<GP_EXCP;
   ctrl_traps_set_update(1);
   dbg_hard_protect();
}

static void dbg_hard_stp_release()
{
   if(dbg_hard_stp_enabled())
      return;

   debug(DBG_HARD_STP, "sstep release\n");
   dbg_hard_stp_restore_context();

   __rflags.tf = dbg_hard_stp_saved_tf();
   __post_access(__rflags);

   /* XXX */
   /* if(!dbg_hard_brk_enabled()) */
   /*    __release_rflags(); */

   info->vmm.ctrl.dbg.excp &= ~(1<<GP_EXCP);
   ctrl_traps_set_update(1);
   dbg_hard_release();
}

static int dbg_hard_stp_event_fast_syscall(int tf)
{
   int    rc;
   size_t sz;

   dbg_hard_stp_restore_context();

   sz = ud_insn_len(&info->vm.cpu.disasm);
   rc = emulate_done(emulate_insn(&info->vm.cpu.disasm), sz);
   info->vm.cpu.emu_sts = EMU_STS_AVL; /* stealth for db_pending() */

   dbg_hard_stp_setup_context();

   if(rc == VM_DONE_LET_RIP)
   {
      __rflags.tf = tf;
      __post_access(__rflags);
   }

   return rc;
}

/*
** Hardware Single-step Services
*/
int dbg_hard_stp_event_gp()
{
   size_t mn;

   if(!dbg_hard_stp_enabled())
      return VM_IGNORE;

   debug(DBG_HARD_STP, "sstep #GP event\n");

   if(!disassemble(&info->vm.cpu.disasm))
      return VM_FAIL;

   mn = info->vm.cpu.disasm.mnemonic;
   switch(mn)
   {
   case UD_Isysenter: return dbg_hard_stp_event_fast_syscall(0);
   case UD_Isysexit:  return dbg_hard_stp_event_fast_syscall(1);
   }

   return VM_IGNORE;
}

int dbg_hard_stp_event()
{
   offset_t  addr;
   int       mode;
   dbg_evt_t *evt;

   if(!dbg_hard_stp_enabled())
      return VM_IGNORE;

   debug(DBG_HARD_STP, "sstep event [req %s]\n"
	 ,dbg_hard_stp_requestor()?"vmm":"usr");

   if(dbg_soft_resuming())
      dbg_soft_resume_post();

   dbg_hard_stp_disable();

   if(dbg_hard_stp_requestor() == DBG_REQ_VMM)
   {
      dbg_hard_dr6_clean();
      return VM_INTERN;
   }

   dbg_hard_set_dr6_dirty(1);
   vm_get_code_addr(&addr, 0, &mode);

   evt = &info->vmm.ctrl.dbg.evt;
   evt->type = DBG_EVT_TYPE_HARD_SSTEP;
   evt->addr = addr;

   debug(DBG_HARD_STP, "prepared sstep ctrl event for 0x%X\n", evt->addr);
   return VM_DONE;
}

void dbg_hard_stp_enable(uint8_t req)
{
   if(dbg_hard_stp_enabled())
      return;

   debug(DBG_HARD_STP, "sstep enable\n");

   dbg_hard_stp_set_enable(1);
   dbg_hard_stp_set_req(req);
   dbg_hard_stp_protect();
}

void dbg_hard_stp_disable()
{
   if(!dbg_hard_stp_enabled())
      return;

   debug(DBG_HARD_STP, "sstep disable\n");

   dbg_hard_stp_set_enable(0);
   dbg_hard_stp_release();
}

void dbg_hard_stp_reset()
{
   dbg_hard_stp_disable();
   info->vmm.ctrl.dbg.hard.stp.sts.raw = 0;
}
