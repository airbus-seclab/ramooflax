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
#ifndef __GDB_STUB_CMD_H__
#define __GDB_STUB_CMD_H__

#include <types.h>
#include <gdbstub.h>

#define GDB_CMD_THREAD         'H'
#define GDB_CMD_STOP_REASON    '?'
#define GDB_CMD_R_GPR          'g'
#define GDB_CMD_W_GPR          'G'
#define GDB_CMD_R_REG          'p'
#define GDB_CMD_W_REG          'P'
#define GDB_CMD_R_MEM          'm'
#define GDB_CMD_W_MEM          'M'
#define GDB_CMD_CONT           'c'
#define GDB_CMD_STEP           's'
#define GDB_CMD_KILL           'k'
#define GDB_CMD_DETACH         'D'

#define GDB_CMD_S_BRK          'Z'
#define GDB_CMD_R_BRK          'z'

#define GDB_BRK_TYPE_MEM        0
#define GDB_BRK_TYPE_HRD_X      1
#define GDB_BRK_TYPE_HRD_W      2
#define GDB_BRK_TYPE_HRD_RW     4

#define GDB_CMD_QUERY          'q'
#define GDB_QUERY_THREAD       'C'


/*
** Functions
*/
void gdb_process_packet(uint8_t*, size_t);

#endif
