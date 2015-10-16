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
#ifndef __GDB_STUB_REG_H__
#define __GDB_STUB_REG_H__

#include <types.h>
#include <gdbstub.h>

/*
** GDB register access
*/
typedef void (*gdb_reg_acc_fnc_t)(loc_t*, size_t*, uint8_t, uint8_t);

typedef struct gdb_register_access_entry
{
   char              *name;
   gdb_reg_acc_fnc_t handler;

} __attribute__((packed)) gdb_reg_acc_e_t;

typedef struct gdb_register_accessor
{
   gdb_reg_acc_e_t  *reg;
   size_t           nr;

} __attribute__((packed)) gdb_reg_acc_t;

/*
** Functions
*/
int __gdb_setup_reg(uint64_t, raw64_t**, size_t*, uint8_t, uint8_t);

#endif
