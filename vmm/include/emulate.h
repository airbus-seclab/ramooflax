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
#ifndef __EMULATE_H__
#define __EMULATE_H__

#include <types.h>
#include <asm.h>
#include <disasm.h>

/*
** emulation return codes
*/
#define EMU_FAIL              0
#define EMU_FAULT             1
#define EMU_SUCCESS           2
#define EMU_SUCCESS_LET_RIP   3
#define EMU_UNSUPPORTED       4

/*
** Functions
*/
int   emulate();
void  emulate_unsupported(ud_t*);
int   emulate_int15();

int   __emulate_insn(ud_t*);
int   __emulate_exception(uint8_t);
int   __emulate_hard_interrupt(uint8_t);
int   __emulate_soft_interrupt(uint8_t);

#endif

