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
#ifndef __GDB_STUB_VMM_H__
#define __GDB_STUB_VMM_H__

#include <types.h>

#define GDB_CMD_VMM    5

typedef void (*gdb_vmm_hdl_t)(uint8_t*, size_t);

/*
** Functions
*/
void  gdb_cmd_vmm(uint8_t*, size_t);
void  gdb_vmm_enable();
void  gdb_vmm_disable();

#endif
