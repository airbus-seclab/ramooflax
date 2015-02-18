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
#include <gdbstub.h>
#include <gdbstub_pkt.h>
#include <gdbstub_vmm.h>
#include <dbg.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

void gdb_disable()
{
   gdb_vmm_disable();
   dbg_disable();

   gdb_set_enable(0);
   gdb_set_lock(0);
}

void gdb_enable()
{
   gdb_set_last_stop_reason(GDB_EXIT_INT);
   gdb_set_enable(1);
   gdb_set_lock(1);

   dbg_enable();
   gdb_vmm_enable();
}

void gdb_reset()
{
   gdb_disable();
}

int __gdb_preempt(uint8_t reason)
{
   gdb_send_stop_reason(reason);
   gdb_set_last_stop_reason(reason);
   gdb_set_lock(1);
   return 1;
}

int gdb_preempt(uint8_t reason)
{
   /* check if we have a request to process first */
   gdb_recv_packet();
   return __gdb_preempt(reason);
}

void gdbstub()
{
   if(gdb_enabled() || (info->vmm.ctrl.vmexit_cnt.raw % GDB_RATIO) == 0)
      gdb_recv_packet();
}
