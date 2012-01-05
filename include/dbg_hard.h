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
#ifndef __DBG_HARD_H__
#define __DBG_HARD_H__

#include <types.h>
#include <dbg_hard_brk.h>
#include <dbg_hard_stp.h>

typedef union dbg_hard_status
{
   struct
   {
      uint8_t    dr6:1;  /* must clean dr6 */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) dbg_hard_sts_t;


typedef struct dbg_hard
{
   dbg_hard_sts_t  sts;
   dbg_hard_brk_t  brk;
   dbg_hard_stp_t  stp;

} __attribute__((packed)) dbg_hard_t;

/*
** Functions
*/
#define ___dbg_hard                (info->vmm.ctrl.dbg.hard)
#define dbg_hard_dr6_dirty()       (___dbg_hard.sts.dr6)
#define dbg_hard_set_dr6_dirty(_x) (___dbg_hard.sts.dr6=(_x))
#define dbg_hard_dr6_clean()			\
   ({                                           \
      dbg_hard_set_dr6_dirty(0);		\
      __dr6.wlow = 0x0ff0;                      \
      __post_access(__dr6);                     \
   })

void dbg_hard_setup();
void dbg_hard_reset();
void dbg_hard_protect();
void dbg_hard_release();

#endif
