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
#ifndef __GDB_SSTEP_H__
#define __GDB_SSTEP_H__

#include <types.h>
#include <cr.h>
#include <msr.h>

#define GDB_SSTEP_USER   0
#define GDB_SSTEP_VMM    1

typedef union vmm_ctrl_dbg_single_step_status
{
   struct
   {
      uint8_t    on:1;    /* sstep active */
      uint8_t    tf:1;    /* saved vm rflags.tf */
      uint8_t    rq:1;    /* requested by */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) vmm_ctrl_dbg_stp_status_t;

typedef struct vmm_ctrl_dbg_single_step_context
{
   cr3_reg_t        cr3; /* XXX: maybe useless */
   raw_msr_entry_t  sysenter_cs;

} __attribute__((packed)) vmm_ctrl_dbg_stp_ctx_t;

typedef struct vmm_ctrl_dbg_single_step
{
   vmm_ctrl_dbg_stp_status_t status;
   vmm_ctrl_dbg_stp_ctx_t    ctx;

} __attribute__((packed)) vmm_ctrl_dbg_stp_t;

#define gdb_singlestep_set_requestor(_x)	\
   ({						\
      info->vmm.ctrl.dbg.stp.status.rq = (_x);	\
   })

#define gdb_singlestep_requestor()      (info->vmm.ctrl.dbg.stp.status.rq)
#define gdb_singlestep_enabled()        (info->vmm.ctrl.dbg.stp.status.on)

/*
** Functions
*/
int   gdb_singlestep();
int   gdb_singlestep_fake();
int __gdb_singlestep_fake();
int   gdb_singlestep_enable(uint8_t);
void  gdb_singlestep_disable();
int   gdb_singlestep_check();
void  gdb_singlestep_set_rflags(rflags_reg_t*);
void  gdb_singlestep_restore_rflags(rflags_reg_t*);
int   gdb_singlestep_correct_rflags(rflags_reg_t*);

#endif
