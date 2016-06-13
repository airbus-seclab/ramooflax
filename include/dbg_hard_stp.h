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
#ifndef __DBG_HARD_STP_H__
#define __DBG_HARD_STP_H__

#include <types.h>
#include <cr.h>
#include <msr.h>
#include <ctrl_evt.h>

typedef union dbg_hardware_singlestep_status
{
   struct
   {
      uint8_t    on:1;    /* sstep active */
      uint8_t    tf:1;    /* saved vm rflags.tf */
      uint8_t    rq:1;    /* requested by */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) dbg_hard_stp_sts_t;

typedef struct dbg_hardware_singlestep_context
{
   msr_t     sysenter_cs;
   /* XXX: need syscall/sysret support too */

} __attribute__((packed)) dbg_hard_stp_ctx_t;

typedef struct dbg_hardware_singlestep
{
   dbg_hard_stp_sts_t sts;
   dbg_hard_stp_ctx_t ctx;

} __attribute__((packed)) dbg_hard_stp_t;

/*
** Functions
*/
#define ___hstp                          (info->vmm.ctrl.dbg.hard.stp)
#define __hstp_ctx                       (___hstp.ctx)

#define dbg_hard_stp_enabled()           (___hstp.sts.on)
#define dbg_hard_stp_saved_tf()          (___hstp.sts.tf)
#define dbg_hard_stp_requestor()         (___hstp.sts.rq)

#define dbg_hard_stp_set_enable(_x)      (___hstp.sts.on=(_x))
#define dbg_hard_stp_save_tf(_x)         (___hstp.sts.tf=(_x))
#define dbg_hard_stp_set_req(_x)         (___hstp.sts.rq=(_x))

void dbg_hard_stp_enable(uint8_t);
void dbg_hard_stp_disable();
void dbg_hard_stp_reset();

void dbg_hard_stp_setup_context();
void dbg_hard_stp_restore_context();

int  dbg_hard_stp_fake();
int  dbg_hard_stp_event_gp();
int  dbg_hard_stp_event();

#endif
