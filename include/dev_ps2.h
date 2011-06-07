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
#ifndef __DEV_PS2_SYS_CTRL_H__
#define __DEV_PS2_SYS_CTRL_H__

#include <types.h>
#include <io.h>

#define PS2_SYS_CTRL_PORT_A     0x92

/*
** PS2 System Controler Chip
** mainly used to fast enable A20 gate
*/
typedef union ps2_system_control
{
   struct
   {
      uint8_t  fast_reset:1;
      uint8_t  enabled_a20:1;
      uint8_t  other:6;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) ps2_t;

/*
** Functions
*/
int  dev_ps2(ps2_t*, io_insn_t*);



#endif
