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
#ifndef __DBG_SOFT_H__
#define __DBG_SOFT_H__

#include <types.h>
#include <cr.h>
#include <ctrl_evt.h>

/*
** Software (memory) breakpoints
*/
typedef union dbg_software_breakpoint_status
{
   struct
   {
      uint8_t  rq:1;  /* requested by */
      uint8_t  on:1;  /* still active */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) dbg_soft_bp_sts_t;

typedef struct dbg_software_breakpoint
{
   dbg_soft_bp_sts_t  sts;  /* brk status */
   offset_t           addr; /* brk location */
   ctrl_evt_hdl_t     hdlr; /* specific handler */
   uint8_t            byte; /* saved insn byte */

} __attribute__((packed)) dbg_soft_bp_t;

typedef union dbg_software_status
{
   struct
   {
      uint8_t  on:1;    /* soft bp enabled */
      uint8_t  rs:1;    /* resume over bp */
      uint8_t  dis:1;   /* disarm all bp */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) dbg_soft_sts_t;

#define DBG_SOFT_NR  20

typedef struct dbg_software
{
   dbg_soft_sts_t sts;
   size_t         cnt;
   dbg_soft_bp_t  list[DBG_SOFT_NR];

} __attribute__((packed)) dbg_soft_t;

/*
** Functions
*/
typedef int (*dbg_soft_hdl_t)(dbg_soft_bp_t*, void*);

#define ___dbg_soft                 (info->vmm.ctrl.dbg.soft)
#define dbg_soft_enabled()          (___dbg_soft.sts.on)
#define dbg_soft_disarmed()         (___dbg_soft.sts.dis)
#define dbg_soft_resuming()         (___dbg_soft.sts.rs)

#define dbg_soft_set_enable(_x)     (___dbg_soft.sts.on=(_x))
#define dbg_soft_set_resume(_x)     (___dbg_soft.sts.rs=(_x))
#define dbg_soft_set_disarm(_x)     (___dbg_soft.sts.dis=(_x))

void dbg_soft_enable();
void dbg_soft_disable();
void dbg_soft_reset();

int  dbg_soft_set(offset_t, ctrl_evt_hdl_t);
int  dbg_soft_del(offset_t);
int  dbg_soft_event(ctrl_evt_hdl_t*);
int  dbg_soft_resume();
int  dbg_soft_resume_post(cr3_reg_t*);
int  dbg_soft_disarm();
int  dbg_soft_rearm();

#endif
