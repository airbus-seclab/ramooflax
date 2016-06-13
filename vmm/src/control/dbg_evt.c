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
#include <dbg_evt.h>
#include <ctrl_evt.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

int dbg_evt_soft()
{
   ctrl_evt_hdl_t hdlr;
   int            rc = dbg_soft_event(&hdlr);

   if(rc == VM_DONE)
   {
      arg_t arg;
      arg.addr = &info->vmm.ctrl.dbg.evt;
      ctrl_evt_setup(CTRL_EVT_USR_TYPE_BRK, hdlr, arg);
   }

   return rc;
}

int dbg_evt_hard()
{
   ctrl_evt_hdl_t hdlr;
   uint8_t        type;
   int            rc;

   __pre_access(__dr6);

   if(__dr6.bs)
   {
      type = CTRL_EVT_USR_TYPE_SSTEP;
      hdlr = 0;
      rc   = dbg_hard_stp_event();
   }
   else
   {
      type = CTRL_EVT_USR_TYPE_BRK;
      rc = dbg_hard_brk_event(&hdlr);
   }

   if(rc == VM_DONE)
   {
      arg_t arg;
      arg.addr = &info->vmm.ctrl.dbg.evt;
      ctrl_evt_setup(type, hdlr, arg);
   }

   debug(DBG_EVT, "dbg_evt %d rc = %d\n", type, rc);
   return rc;
}

int dbg_evt_gp()
{
   debug(DBG_EVT, "dbg_evt #GP\n");
   return dbg_hard_stp_event_gp();
}
