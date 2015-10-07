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
#include <db.h>
#include <insn.h>
#include <excp.h>
#include <debug.h>
#include <emulate.h>
#include <info_data.h>

extern info_data_t *info;

static void ctrl_traps_enable()
{
   debug(CTRL, "ctrl traps enable (%d|%d)\n"
	 ,ctrl_traps_enabled(), ctrl_traps_updated());

   if(dbg_hard_stp_enabled())
      dbg_hard_stp_setup_context();

   dbg_soft_rearm();
   dbg_hard_brk_rearm();

   __update_exception_mask();

   ctrl_set_traps(1);
   ctrl_traps_set_update(0);
}

static void ctrl_traps_disable()
{
   debug(CTRL, "ctrl traps disable (%d|%d)\n"
	 ,ctrl_traps_enabled(), ctrl_traps_updated());

   if(dbg_hard_stp_enabled())
      dbg_hard_stp_restore_context();

   if(!dbg_soft_disarmed())
      dbg_soft_disarm();

   dbg_hard_brk_disarm();

   __exception_bitmap.raw = info->vm.cpu.dflt_excp;
   __post_access(__exception_bitmap);

   ctrl_set_traps(0);
   ctrl_traps_set_update(0);
}

static void ctrl_traps()
{
#ifdef CONFIG_CTRL_DBG
   int up=ctrl_traps_updated();
#endif

   if(__ctrl_active_cr3_check(0))
   {
      if(!ctrl_traps_enabled() || ctrl_traps_updated())
	 ctrl_traps_enable();
   }
   else if(ctrl_traps_enabled() || ctrl_traps_updated())
      ctrl_traps_disable();

#ifdef CONFIG_CTRL_DBG
   if(up)
      debug(CTRL, "ctrl traps state: en:%d hw:%d stp:%d soft:%d\n"
	    ,ctrl_traps_enabled()
	    ,!dbg_hard_brk_disarmed(), dbg_hard_stp_enabled()
	    ,!dbg_soft_disarmed());
#endif
}

int __ctrl_active_cr3_check(int rpl)
{
   if(!ctrl_cr3_enabled())
      return 1;

   if(__cpl < rpl)
      return 0;

   if(_xx_lmode())
      return (__cr3.pml4.addr == info->vmm.ctrl.stored_cr3.pml4.addr);
   else if(__cr4.pae)
      return (__cr3.pae.addr == info->vmm.ctrl.stored_cr3.pae.addr);
   else
      return (__cr3.addr == info->vmm.ctrl.stored_cr3.addr);

   return 0;
}

void ctrl_active_cr3_enable(cr3_reg_t cr3)
{
   if(ctrl_cr3_enabled())
      ctrl_active_cr3_disable();

   info->vmm.ctrl.stored_cr3.raw = cr3.raw;
   info->vmm.ctrl.active_cr3 = &info->vmm.ctrl.stored_cr3;
   ctrl_traps_set_update(1);
   ctrl_set_cr3(1);

   debug(CTRL, "active cr3: 0x%X (0x%X <=> 0x%X)\n"
	 , info->vmm.ctrl.stored_cr3.raw
	 , info->vmm.ctrl.active_cr3
	 ,&info->vmm.ctrl.stored_cr3);
}

void ctrl_active_cr3_disable()
{
   dbg_soft_reset();

   info->vmm.ctrl.active_cr3 = &__cr3;
   ctrl_traps_set_update(1);
   ctrl_set_cr3_keep(0);
   ctrl_set_cr3(0);
}

void ctrl_active_cr3_reset()
{
   if(!ctrl_cr3_keep())
      ctrl_active_cr3_disable();
}

void ctrl_usr_reset()
{
   info->vmm.ctrl.usr.excp = 0;
   __update_exception_mask();

   ctrl_usr_set_cr_rd(0);
   ctrl_usr_set_cr_wr(0);

   __disable_lbr();

   debug(CTRL, "ctrl user filters disabled\n");
}

int controller()
{
   int rc = VM_IGNORE;

   if(ctrl_active_cr3_check())
      rc = ctrl_event();

#ifdef CONFIG_GDBSTUB
   gdbstub();
#endif

   if(rc & (VM_FAIL|VM_FAULT))
      return rc;

   if(dbg_hard_dr6_dirty())
      dbg_hard_dr6_clean();

   ctrl_traps();

   return rc;
}
