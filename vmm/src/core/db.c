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
#include <db.h>
#include <gdb.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void db_check_native_sstep()
{
   /*
   ** XXX:
   ** . iret, popf, mov ss ... we should not inject, the cpu
   **   will do it one insn later due to TF. It's the normal
   **   behavior for these instructions
   */
   if(__rflags.tf &&
      !gdb_singlestep_check() &&
      __vmexit_on_insn())
   {
      debug(DB, "native singlestep\n");
      __inject_exception(DB_EXCP, 0, 0);
   }
}

void db_post_hdl()
{
   if(gdb_dr6_is_dirty())
      gdb_clean_dr6();

   if(!__injecting_exception())
      db_check_native_sstep();
}
