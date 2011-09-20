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
#ifndef __GDB_CORE_H__
#define __GDB_CORE_H__

#include <types.h>

/*
** Functions
*/
int    gdb_excp_event(uint32_t);
void   gdb_stub_post();

int    __gdb_active_cr3_check(int);
#define gdb_active_cr3_check() __gdb_active_cr3_check(3)

void   gdb_traps_disable();
void   gdb_traps_enable();

void   gdb_post_inspect_rflags();

void   gdb_protect_bp_excp();
void   gdb_release_bp_excp();
void   gdb_protect_db_excp();
void   gdb_release_db_excp();

int    gdb_cr_rd_event(uint8_t);
int    gdb_cr_wr_event(uint8_t);

#endif
