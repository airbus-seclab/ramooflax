/*
** Copyright (C) 2016 Airbus Group, stephane duverger <stephane.duverger@airbus.com>
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
#include <db.h>
#include <ctrl.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int db_check_pending_stp()
{
   if(!__rflags.tf)
      return VM_IGNORE;

   __rflags.tf = 0;
   __post_access(__rflags);

   __pre_access(__dr6);
   __dr6.bs = 1;

   /* XXX: priority, pending #DB ??? */
   if(__injecting_event())
   {
      debug(DB, "already injecting event, missing #DB.sstep\n");
      dbg_hard_set_dr6_dirty(1);
      return VM_DONE;
   }

   __inject_exception(DB_EXCP, 0, 0);
   __post_access(__dr6);
   return VM_DONE;
}

/*
** Hardware exec traps are checked before
** insn execution. But hardware data, i/o
** and single-step traps are checked after.
**
** If we emulated an insn, we may loose
** a #DB condition, so take care here.
**
** XXX: iret, popf, mov ss ...
*/
void db_check_pending()
{
   if(!__vmexit_on_insn())
      return;

   /* XXX: missing data/io */
   db_check_pending_stp();
}
