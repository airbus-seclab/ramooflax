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
#include <emulate.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void dbg_hard_stp_save_context()
{
   __hstp_ctx.cr3.raw = __cr3.raw;
   __pre_access(__sysenter_cs);
   __hstp_ctx.sysenter_cs.raw = __sysenter_cs.raw;
}

void dbg_hard_stp_setup_context()
{
   debug(DBG_HARD_STP, "stp install sysenter\n");
   __sysenter_cs.raw = 0;
   __post_access(__sysenter_cs);
}

void dbg_hard_stp_restore_context()
{
   debug(DBG_HARD_STP, "stp restored sysenter\n");
   __sysenter_cs.raw = __hstp_ctx.sysenter_cs.raw;
   __post_access(__sysenter_cs);
}

static void dbg_hard_stp_protect()
{
   if(!dbg_hard_stp_enabled())
      return;

   debug(DBG_HARD_STP, "hard stp protect\n");

   dbg_hard_stp_save_context();

   dbg_hard_stp_save_tf(__rflags.tf);
   __rflags.tf = 1;
   __post_access(__rflags);

   /* XXX */
   /* __protect_rflags(); */

   info->vmm.ctrl.dbg.excp |= 1<<GP_EXCP;
   dbg_hard_protect();
}

static void dbg_hard_stp_release()
{
   if(dbg_hard_stp_enabled())
      return;

   debug(DBG_HARD_STP, "hard stp release\n");

   dbg_hard_stp_restore_context();

   __rflags.tf = dbg_hard_stp_saved_tf();
   __post_access(__rflags);

   /* XXX */
   /* if(!dbg_hard_brk_enabled()) */
   /*    __release_rflags(); */

   info->vmm.ctrl.dbg.excp &= ~(1<<GP_EXCP);
   dbg_hard_release();
}

static int dbg_hard_stp_event_sysenter()
{
   __rflags.tf = 0;
   __post_access(__rflags);
   return CTRL_EVT_DONE;
}

static int dbg_hard_stp_event_sysexit()
{
   __rflags.tf = 1;
   __post_access(__rflags);
   return CTRL_EVT_DONE;
}

/*
** Hardware Single-step Services
*/
int dbg_hard_stp_event_gp()
{
   ud_t disasm;
   int  rc;

   if(!dbg_hard_stp_enabled())
      return CTRL_EVT_IGNORE;

   debug(DBG_HARD_STP, "sstep #GP event\n");

   dbg_hard_stp_restore_context();

   if(!disassemble(&disasm))
      return CTRL_EVT_FAIL;

   rc = __emulate_insn(&disasm);
   dbg_hard_stp_setup_context();

   switch(rc)
   {
   case EMU_FAULT:       return CTRL_EVT_FAULT;
   case EMU_UNSUPPORTED: return CTRL_EVT_IGNORE;
   case EMU_FAIL:        return CTRL_EVT_FAIL;
   }

   if(disasm.mnemonic == UD_Isysenter)
      return dbg_hard_stp_event_sysenter();

   if(disasm.mnemonic == UD_Isysexit)
      return dbg_hard_stp_event_sysexit();

   return CTRL_EVT_IGNORE;
}

int dbg_hard_stp_event()
{
   offset_t  addr;
   int       mode;
   dbg_evt_t *evt;

   if(!dbg_hard_stp_enabled())
      return CTRL_EVT_IGNORE;

   debug(DBG_HARD_STP, "sstep event\n");

   if(dbg_soft_resuming())
      dbg_soft_resume_post(&__hstp_ctx.cr3);

   dbg_hard_set_dr6_dirty(1);
   dbg_hard_stp_disable();

   if(dbg_hard_stp_requestor() == DBG_REQ_VMM)
   {
      debug(DBG_HARD_STP, "internal sstep event\n");
      return CTRL_EVT_INTERN;
   }

   vm_get_code_addr(&addr, 0, &mode);

   evt = &info->vmm.ctrl.dbg.evt;
   evt->type = DBG_EVT_TYPE_HARD_SSTEP;
   evt->addr = addr;

   debug(DBG_HARD_STP, "prepared sstep ctrl event for 0x%X\n", evt->addr);
   return CTRL_EVT_DONE;
}

void dbg_hard_stp_enable(uint8_t req)
{
   if(dbg_hard_stp_enabled())
      return;

   debug(DBG_HARD_STP, "hard stp enable\n");

   dbg_hard_stp_set_enable(1);
   dbg_hard_stp_set_req(req);
   dbg_hard_stp_protect();
}

void dbg_hard_stp_disable()
{
   if(!dbg_hard_stp_enabled())
      return;

   debug(DBG_HARD_STP, "hard stp disable\n");

   dbg_hard_stp_set_enable(0);
   dbg_hard_stp_release();
}

void dbg_hard_stp_reset()
{
   dbg_hard_stp_disable();
   info->vmm.ctrl.dbg.hard.stp.sts.raw = 0;
}
