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
#ifndef __GDB_BRK_H__
#define __GDB_BRK_H__

#include <types.h>
#include <dr.h>

#define GDB_DFLT_BRK_NR    20
#define GDB_BRK_USER        0
#define GDB_BRK_VMM         1

typedef union vmm_ctrl_dbg_brk_mem_setup
{
   struct
   {
      uint8_t  rq:1;  /* requested by */
      uint8_t  on:1;  /* still active */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) vmm_ctrl_dbg_brk_mem_setup_t;

typedef struct vmm_ctrl_dbg_brk_mem_entry
{
   offset_t                     addr;  /* breakpoint location */
   uint8_t                      byte;  /* saved insn byte */
   vmm_ctrl_dbg_brk_mem_setup_t setup; /* this mem brk setup */

} __attribute__((packed)) vmm_ctrl_dbg_brk_mem_e_t;

typedef union vmm_ctrl_dbg_brk_mem_status
{
   struct
   {
      uint8_t  rs:1;    /* need to restore mem bp */
      uint8_t  dis:1;   /* all mem bp are disarmed */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) vmm_ctrl_dbg_brk_mem_status_t;

#define gdb_brk_mem_reset_status()   (info->vmm.ctrl.dbg.brk.mem.status.raw = 0)
#define gdb_brk_mem_disarmed()       (info->vmm.ctrl.dbg.brk.mem.status.dis)

typedef struct vmm_ctrl_dbg_breakpoints_memory
{
   vmm_ctrl_dbg_brk_mem_status_t status;
   vmm_ctrl_dbg_brk_mem_e_t      list[GDB_DFLT_BRK_NR];
   vmm_ctrl_dbg_brk_mem_e_t      *last;
   offset_t                      last_addr;

} __attribute__((packed)) vmm_ctrl_dbg_brk_mem_t;

typedef union vmm_ctrl_dbg_brk_hrd_status
{
   struct
   {
      uint8_t    on:1;      /* hrd breakpoints installed */
      uint8_t    insn:1;    /* hrd insn breakpoints installed */
      uint8_t    last_n:2;  /* last triggered hrd breakpoint index */
      uint8_t    last_t:3;  /* last triggered hrd breakpoint type */
      uint8_t    rf:1;      /* saved vm rflags.rf */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) vmm_ctrl_dbg_brk_hrd_status_t;

typedef struct vmm_ctrl_dbg_breakpoints_hardware
{
   vmm_ctrl_dbg_brk_hrd_status_t status;
   dr7_reg_t                     dr7;

} __attribute__((packed)) vmm_ctrl_dbg_brk_hrd_t;

typedef struct vmm_ctrl_dbg_breakpoints
{
   vmm_ctrl_dbg_brk_hrd_t    hrd;
   vmm_ctrl_dbg_brk_mem_t    mem;

} __attribute__((packed)) vmm_ctrl_dbg_brk_t;

#define gdb_brk_hrd_set_last(_n,_t)			\
   ({							\
      info->vmm.ctrl.dbg.brk.hrd.status.last_n = (_n);	\
      info->vmm.ctrl.dbg.brk.hrd.status.last_t = (_t);	\
   })

#define gdb_brk_hrd_last_idx()       (info->vmm.ctrl.dbg.brk.hrd.status.last_n)
#define gdb_brk_hrd_last_type()      (info->vmm.ctrl.dbg.brk.hrd.status.last_t)

#define gdb_brk_hrd_insn_enabled()   (info->vmm.ctrl.dbg.brk.hrd.status.insn)
#define gdb_brk_hrd_enabled()        (info->vmm.ctrl.dbg.brk.hrd.status.on)

#define gdb_brk_hrd_reset_status()   (info->vmm.ctrl.dbg.brk.hrd.status.raw = 0)

#define gdb_dr6_is_dirty()           (info->vmm.ctrl.dbg.status.dr6)
#define gdb_dr6_set_dirty(_x)        (info->vmm.ctrl.dbg.status.dr6 = (_x))

#define gdb_clean_dr6()				\
   ({						\
      gdb_dr6_set_dirty(0);			\
      __dr6.wlow = 0x0ff0;			\
      __post_access(__dr6);			\
   })

#define gdb_clean_dr7()					\
   ({							\
      info->vmm.ctrl.dbg.brk.hrd.dr7.low &= 0x100;	\
      __dr7.low &= 0x100;				\
      __post_access(__dr7);				\
   })

#define gdb_brk_mem_need_restore()   (info->vmm.ctrl.dbg.brk.mem.status.rs)

#define gdb_brk_mem_set_restore(_x)			\
   ({							\
      info->vmm.ctrl.dbg.brk.mem.status.rs = (_x);	\
   })

/*
** Some restore/insert status
*/
#define GDB_BP_FAIL      0
#define GDB_BP_OK        1
#define GDB_BP_IGNORED   2

/*
** Functions
*/
int   gdb_brk_mem();
void  gdb_brk_mem_set(offset_t);
void  gdb_brk_mem_del(offset_t);
void  gdb_brk_mem_cleanup();
int   gdb_brk_mem_continue();
int   gdb_brk_mem_restore_bp();
int   gdb_brk_mem_restore_insn();
void  __gdb_brk_mem_disarm();
void  __gdb_brk_mem_rearm();

int   gdb_brk_hrd();
void  gdb_brk_hrd_set(offset_t, uint64_t, size_t);
void  gdb_brk_hrd_del(offset_t, uint64_t, size_t);
void  gdb_brk_hrd_enable();
void  gdb_brk_hrd_disable();
void  gdb_brk_hrd_cleanup();
int   gdb_brk_hrd_condition();
int   gdb_brk_hrd_special();
int   gdb_brk_hrd_continue();

void  gdb_brk_hrd_insn_set_rflags(rflags_reg_t*);
void  gdb_brk_hrd_insn_restore_rflags(rflags_reg_t*);
int   gdb_brk_hrd_insn_correct_rflags(rflags_reg_t*);
void  gdb_brk_hrd_insn_enable();
void  gdb_brk_hrd_insn_disable();

int   gdb_brk_continue();
void  gdb_brk_enable();
void  gdb_brk_disable();

#endif
