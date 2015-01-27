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
#include <dbg_soft.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int __dbg_soft_restore_insn(dbg_soft_bp_t *bp)
{
   if(!ctrl_mem_write(bp->addr, &bp->byte, 1))
   {
      debug(DBG_SOFT, "restore insn @ 0x%X failed\n", bp->addr);
      return VM_FAIL;
   }

   bp->sts.st = 0;
   debug(DBG_SOFT, "restored insn @ 0x%X\n", bp->addr);
   return VM_DONE;
}

static int __dbg_soft_restore_bp(dbg_soft_bp_t *bp, cr3_reg_t *cr3)
{
   uint8_t brk_insn = BREAKPOINT_INSN;

   if(!ctrl_mem_write_with(cr3, bp->addr, &brk_insn, 1))
   {
      debug(DBG_SOFT, "restore soft bp @ 0x%X failed\n", bp->addr);
      return VM_FAIL;
   }

   bp->sts.st = 1;
   debug(DBG_SOFT, "restored soft bp @ 0x%X\n", bp->addr);
   return VM_DONE;
}

static int dbg_soft_restore_insn(dbg_soft_bp_t *bp, void __unused__ *arg)
{
   if(!bp->sts.on || !bp->sts.st)
      return VM_IGNORE;

   return __dbg_soft_restore_insn(bp);
}

static int dbg_soft_restore_bp(dbg_soft_bp_t *bp, void __unused__ *arg)
{
   if(!bp->sts.on || bp->sts.st)
      return VM_IGNORE;

   if(dbg_soft_resuming() && bp->addr == info->vmm.ctrl.dbg.evt.addr)
      return VM_IGNORE;

   return __dbg_soft_restore_bp(bp, info->vmm.ctrl.active_cr3);
}

static int __dbg_soft_del_bp(dbg_soft_bp_t *bp, void __unused__ *arg)
{
   if(!bp->sts.on)
      return VM_IGNORE;

   bp->sts.on = 0;

   if(!__dbg_soft_restore_insn(bp))
      return VM_FAIL;

   debug(DBG_SOFT, "del soft bp @ 0x%X\n", bp->addr);
   return VM_DONE;
}

static int dbg_soft_set_bp(dbg_soft_bp_t *bp, void *arg)
{
   offset_t addr = *(offset_t*)arg;

   if(bp->sts.on && bp->addr != addr)
      return VM_IGNORE;

   if(!bp->sts.on)
   {
      bp->addr   = addr;
      bp->sts.on = 1;

      if(!ctrl_mem_read(addr, &bp->byte, 1))
      {
	 debug(DBG_SOFT, "set soft bp @ 0x%X failed\n", addr);
	 return VM_FAIL;
      }

      debug(DBG_SOFT, "set soft bp @ 0x%X\n", addr);
   }

   *(dbg_soft_bp_t**)arg = bp;
   return VM_DONE;
}

static int dbg_soft_del_bp(dbg_soft_bp_t *bp, void *arg)
{
   offset_t addr = (offset_t)arg;

   if(bp->addr != addr)
      return VM_IGNORE;

   return __dbg_soft_del_bp(bp, 0);
}

static int dbg_soft_check_bp(dbg_soft_bp_t *bp, void *arg)
{
   offset_t addr = *(offset_t*)arg;

   if(bp->sts.on && bp->addr == addr)
   {
      debug(DBG_SOFT, "found soft bp @ 0x%X\n", addr);
      *(dbg_soft_bp_t**)arg = bp;
      return VM_DONE;
   }

   return VM_IGNORE;
}

static int dbg_soft_for_each(dbg_soft_hdl_t hdl, void *arg, int stop)
{
   size_t i;
   int    rc;

   for(i=0 ; i<DBG_SOFT_NR ; i++)
      if((rc=hdl(&info->vmm.ctrl.dbg.soft.list[i], arg)) & stop)
	 break;

   return rc;
}

static int dbg_soft_del_all()
{
   if(dbg_soft_for_each(__dbg_soft_del_bp, 0, VM_FAIL) == VM_FAIL)
   {
      debug(DBG_SOFT, "fail to delete all bp\n");
      return 0;
   }

   info->vmm.ctrl.dbg.soft.cnt = 0;
   return 1;
}

static void dbg_soft_protect()
{
   if(!dbg_soft_enabled())
      return;

   info->vmm.ctrl.dbg.excp |= (1<<BP_EXCP);
   ctrl_traps_set_update(1);
}

static void dbg_soft_release()
{
   if(dbg_soft_enabled())
      return;

   info->vmm.ctrl.dbg.excp &= ~(1<<BP_EXCP);
   ctrl_traps_set_update(1);
}

/*
** Software Breakpoint Services
*/
int dbg_soft_set(offset_t addr, ctrl_evt_hdl_t hdlr)
{
   dbg_soft_bp_t *bp;
   int           stop;

   if(info->vmm.ctrl.dbg.soft.cnt == DBG_SOFT_NR)
      return VM_FAIL;

   stop = VM_DONE|VM_FAIL;

   if(dbg_soft_for_each(dbg_soft_set_bp, &addr, stop) == VM_DONE)
   {
      bp = (dbg_soft_bp_t*)addr;
      bp->hdlr = hdlr;
      info->vmm.ctrl.dbg.soft.cnt++;
      dbg_soft_enable();
      return VM_DONE;
   }

   return VM_FAIL;
}

int dbg_soft_del(offset_t addr)
{
   int stop;

   if(info->vmm.ctrl.dbg.soft.cnt == 0)
      return VM_FAIL;

   stop = VM_DONE|VM_FAIL;

   if(dbg_soft_for_each(dbg_soft_del_bp, (void*)addr, stop) == VM_DONE)
   {
      info->vmm.ctrl.dbg.soft.cnt--;
      if(info->vmm.ctrl.dbg.soft.cnt == 0)
	 dbg_soft_disable();
      return VM_DONE;
   }

   return VM_FAIL;
}

/*
** Usually used on "continue" after dbg_evt is raised
** between dbg_evt and continue, bp might have been removed/changed.
**
** We only restore insn if bp is still installed, to be able to execute
** original instruction.
**
** An internal-single-step must be used to restore bp after insn execution.
*/
int dbg_soft_resume()
{
   dbg_soft_bp_t *bp = info->vmm.ctrl.dbg.evt.soft;

   if(info->vmm.ctrl.dbg.evt.type != DBG_EVT_TYPE_SOFT_BRK)
      return 0;

   if(!bp->sts.on || bp->addr != info->vmm.ctrl.dbg.evt.addr)
      return 0;

   if(__dbg_soft_restore_insn(bp) == VM_DONE)
   {
      dbg_soft_set_resume(1);
      return 1;
   }

   return 0;
}

int dbg_soft_resume_post(cr3_reg_t *cr3)
{
   if(__dbg_soft_restore_bp(info->vmm.ctrl.dbg.evt.soft, cr3) == VM_DONE)
   {
      dbg_soft_set_resume(0);
      return 1;
   }

   return 0;
}

int dbg_soft_disarm()
{
   if(dbg_soft_for_each(dbg_soft_restore_insn, 0, VM_FAIL) == VM_FAIL)
   {
      debug(DBG_SOFT, "fail to disarm bp\n");
      return 0;
   }

   dbg_soft_set_disarm(1);
   return 1;
}

int dbg_soft_rearm()
{
   if(dbg_soft_for_each(dbg_soft_restore_bp, 0, VM_FAIL) == VM_FAIL)
   {
      debug(DBG_SOFT, "fail to rearm bp\n");
      return 0;
   }

   dbg_soft_set_disarm(0);
   return 1;
}

int dbg_soft_event(ctrl_evt_hdl_t *hdlr)
{
   offset_t addr;
   int      mode, stop;

   if(!dbg_soft_enabled())
      return VM_IGNORE;

   debug(DBG_SOFT, "soft brk event\n");

   vm_get_code_addr(&addr, 0, &mode);
   stop = VM_DONE|VM_FAIL;

   if(dbg_soft_for_each(dbg_soft_check_bp, &addr, stop) == VM_DONE)
   {
      dbg_evt_t     *evt = &info->vmm.ctrl.dbg.evt;
      dbg_soft_bp_t *bp  = (dbg_soft_bp_t*)addr;

      evt->type = DBG_EVT_TYPE_SOFT_BRK;
      evt->soft = bp;
      evt->addr = bp->addr;
      *hdlr     = bp->hdlr;

      debug(DBG_SOFT, "prepared soft brk ctrl event for 0x%X\n", evt->addr);
      return VM_DONE;
   }

   return VM_IGNORE;
}

void dbg_soft_enable()
{
   dbg_soft_set_enable(1);
   dbg_soft_protect();
}

void dbg_soft_disable()
{
   dbg_soft_set_enable(0);
   dbg_soft_release();
}

void dbg_soft_reset()
{
   dbg_soft_del_all();
   dbg_soft_disable();
   info->vmm.ctrl.dbg.soft.sts.raw = 0;
}
