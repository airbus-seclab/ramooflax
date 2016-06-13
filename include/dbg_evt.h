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
#ifndef __DBG_EVT_H__
#define __DBG_EVT_H__

#include <types.h>
#include <dbg_soft.h>
#include <dbg_hard.h>

#define DBG_EVT_TYPE_SOFT_BRK      0xff
#define DBG_EVT_TYPE_HARD_BRK_X    DR7_COND_X
#define DBG_EVT_TYPE_HARD_BRK_W    DR7_COND_W
#define DBG_EVT_TYPE_HARD_BRK_IO   DR7_COND_IO
#define DBG_EVT_TYPE_HARD_BRK_RW   DR7_COND_RW
#define DBG_EVT_TYPE_HARD_SSTEP    (DR7_COND_RW+1)

typedef struct dbg_event
{
   uint8_t   type;
   offset_t  addr;

   union
   {
      dbg_soft_bp_t *soft; /* pointer to last raised soft bp */
      uint8_t        hard; /* index of last raised hard bp */

   } __attribute__((packed));

} __attribute__((packed)) dbg_evt_t;

/*
** Functions
*/
int  dbg_evt_soft();
int  dbg_evt_hard();
int  dbg_evt_gp();

#endif
