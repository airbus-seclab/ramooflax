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
#include <dbg_hard_brk.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void dbg_hard_brk_insn_protect()
{
   if(!dbg_hard_brk_insn_enabled())
      return;

   dbg_hard_brk_save_rf(__rflags.rf);
   __rflags.rf = 0;
   __post_access(__rflags);
   /* XXX */
   /* __protect_rflags(); */
}

static void dbg_hard_brk_insn_release()
{
   if(dbg_hard_brk_insn_enabled())
      return;

   __rflags.rf = dbg_hard_brk_saved_rf();
   __post_access(__rflags);

   /* XXX */
   /* if(!dbg_hard_stp_enabled()) */
   /*    __release_rflags(); */
}

static void dbg_hard_brk_insn_enable()
{
   dbg_hard_brk_insn_set_enable(1);
   dbg_hard_brk_insn_protect();
}

static void dbg_hard_brk_insn_disable()
{
   dbg_hard_brk_insn_set_enable(0);
   dbg_hard_brk_insn_release();
}

/*
** Hardware Breakpoints Services
*/
int dbg_hard_brk_set(offset_t addr, uint8_t type, uint8_t len, ctrl_evt_hdl_t hdlr)
{
   uint8_t n;

   for(n=0 ; n<DBG_HARD_BRK_NR ; n++)
      if(!__hbrk_enabled(n))
      {
	 __hbrk_setup_bp(n, type, len);
	 dbg_hard_brk_set_hdlr(n, hdlr);
	 set_dr(n, addr);

	 if(type == DR7_COND_X && !dbg_hard_brk_insn_enabled())
	    dbg_hard_brk_insn_enable();

	 dbg_hard_brk_enable();
	 debug(DBG_HARD_BRK, "set hard bp @ 0x%X\n", get_dr(n));
	 return VM_DONE;
      }

   return VM_IGNORE;
}

int dbg_hard_brk_del(offset_t addr, uint8_t type, uint8_t len)
{
   uint8_t n, more, more_x, check, rc = VM_IGNORE;

   more = more_x = 0;
   check = __hbrk_mke_conf(type,len);

   for(n=0 ; n<DBG_HARD_BRK_NR ; n++)
      if(__hbrk_enabled(n))
      {
	 uint8_t conf = __hbrk_get_config(n);
	 if(conf == check && addr == get_dr(n))
	 {
	    __hbrk_delete_bp(n);
	    rc = VM_DONE;
	    debug(DBG_HARD_BRK, "del hard bp @ 0x%X\n", addr);
	 }
	 else if(!more_x || !more)
	 {
	    if(__hbrk_type_of(conf) == DR7_COND_X)
	       more_x = 1;

	    more = 1;
	 }
      }

   if(!more_x)
   {
      dbg_hard_brk_insn_disable();
      debug(DBG_HARD_BRK, "no more hard insn bp\n");

      if(!more)
      {
	 dbg_hard_brk_disable();
	 debug(DBG_HARD_BRK, "no more hard bp\n");
      }
   }

   return rc;
}

int dbg_hard_brk_resume()
{
   uint8_t n = info->vmm.ctrl.dbg.evt.hard;

   if(info->vmm.ctrl.dbg.evt.type != DBG_EVT_TYPE_HARD_BRK_X || !__hbrk_enabled(n))
      return 0;

   __rflags.rf = 1;
   __post_access(__rflags);
   /* XXX protect rflags ? */

   return 1;
}

void dbg_hard_brk_disarm()
{
   __dr7.low &= 0x100;
   __post_access(__dr7);
}

void dbg_hard_brk_rearm()
{
   __dr7.low = info->vmm.ctrl.dbg.hard.brk.dr7.low;
   __post_access(__dr7);
}

int dbg_hard_brk_event(ctrl_evt_hdl_t *hdlr)
{
   uint8_t n;

   if(!dbg_hard_brk_enabled())
      return VM_IGNORE;

   debug(DBG_HARD_BRK, "hard brk event\n");

   for(n=0 ; n<DBG_HARD_BRK_NR ; n++)
      if(__hbrk_raised(n) && __hbrk_enabled(n))
      {
	 dbg_evt_t *evt = &info->vmm.ctrl.dbg.evt;

	 evt->type = __hbrk_get_type(n);
	 evt->hard = n;
	 evt->addr = get_dr(n);
	 *hdlr     = dbg_hard_brk_get_hdlr(n);

	 dbg_hard_set_dr6_dirty(1);
	 debug(DBG_HARD_BRK, "prepared hard brk ctrl event for 0x%X\n", evt->addr);
	 return VM_DONE;
      }

   return VM_IGNORE;
}

void dbg_hard_brk_enable()
{
   debug(DBG_HARD_BRK, "hard brk enabled\n");
   dbg_hard_brk_set_enable(1);
   dbg_hard_protect();
}

void dbg_hard_brk_disable()
{
   debug(DBG_HARD_BRK, "hard brk disable\n");
   dbg_hard_brk_set_enable(0);
   dbg_hard_release();
}

void dbg_hard_brk_reset()
{
   dbg_hard_brk_disable();
   info->vmm.ctrl.dbg.hard.brk.sts.raw = 0;
}
