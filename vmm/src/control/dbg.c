/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#include <dbg.h>
#include <info_data.h>

extern info_data_t *info;

int dbg_resume(uint8_t stp)
{
   uint8_t rc, who;

   dbg_hard_brk_resume();
   rc = dbg_soft_resume();
   if(!(rc & (VM_DONE|VM_IGNORE)))
      return rc;

   if(stp)
      who = DBG_REQ_USR;
   else if(rc == VM_DONE)
      who = DBG_REQ_VMM;
   else
      return VM_DONE;

   dbg_hard_stp_enable(who);
   return VM_DONE;
}

void dbg_enable()
{
   dbg_hard_setup();
}

void dbg_disable()
{
   dbg_soft_reset();
   dbg_hard_reset();
}
