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
#ifndef __GDB_STUB_H__
#define __GDB_STUB_H__

#include <config.h>
#include <types.h>
#include <ctrl_io.h>
#include <mbi.h>

/*
** Interactivity
*/
#define GDB_DFT_RATE         (10UL)

/*
** Stop reasons
*/
#define GDB_EXIT_EVERY       0
#define GDB_EXIT_INT         2       /* gdb client compatibility */
#define GDB_EXIT_TRAP        5       /* gdb client compatibility */

#define GDB_EXIT_R_CR0      10
#define GDB_EXIT_R_CR2      12
#define GDB_EXIT_R_CR3      13
#define GDB_EXIT_R_CR4      14

#define GDB_EXIT_W_CR0      20
#define GDB_EXIT_W_CR2      22
#define GDB_EXIT_W_CR3      23
#define GDB_EXIT_W_CR4      24

#define GDB_EXIT_EXCP_DE    30
#define GDB_EXIT_EXCP_DB    31
#define GDB_EXIT_EXCP_NMI   32
#define GDB_EXIT_EXCP_BP    33
#define GDB_EXIT_EXCP_OF    34
#define GDB_EXIT_EXCP_BR    35
#define GDB_EXIT_EXCP_UD    36
#define GDB_EXIT_EXCP_NM    37
#define GDB_EXIT_EXCP_DF    38
#define GDB_EXIT_EXCP_MO    39
#define GDB_EXIT_EXCP_TS    40
#define GDB_EXIT_EXCP_NP    41
#define GDB_EXIT_EXCP_SS    42
#define GDB_EXIT_EXCP_GP    43
#define GDB_EXIT_EXCP_PF    44
#define GDB_EXIT_EXCP_RSVD  45
#define GDB_EXIT_EXCP_MF    46
#define GDB_EXIT_EXCP_AC    47
#define GDB_EXIT_EXCP_MC    48
#define GDB_EXIT_EXCP_XF    49

#define GDB_EXIT_HARD_INT   50
#define GDB_EXIT_SOFT_INT   51
#define GDB_EXIT_NPF        52
#define GDB_EXIT_HYP        53
#define GDB_EXIT_CPUID      54

/*
** GDB stub Data structures
*/
typedef union gdbstub_status
{
   struct
   {
      uint8_t    on:1;         /* stub active */
      uint8_t    lock:1;       /* stub locked */
      uint8_t    ack:1;        /* gdb ack sent */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) gdbstub_sts_t;

typedef struct gdbstub
{
   gdbstub_sts_t  sts;     /* stub status       */
   uint8_t        reason;  /* last stop reason  */
   uint64_t       rate;    /* gdb interactivity */

} __attribute__((packed)) gdbstub_t;


/*
** Functions
*/
#define ___gstub                       (info->vmm.gstub)
#define gdb_enabled()                  (___gstub.sts.on)
#define gdb_locked()                   (___gstub.sts.lock)
#define gdb_acked()                    (___gstub.sts.ack)
#define gdb_last_stop_reason(_x)       (___gstub.reason)

#define gdb_set_enable(_x)             (___gstub.sts.on=(_x))
#define gdb_set_lock(_x)               (___gstub.sts.lock=(_x))
#define gdb_set_ack(_x)                (___gstub.sts.ack=(_x))
#define gdb_set_last_stop_reason(_x)   (___gstub.reason=(_x))

#define gdb_io_read(_b,_l)             ctrl_io_read(_b,_l)
#define gdb_io_write(_b,_l)            ctrl_io_write(_b,_l)

#define gdb_pmem_recv(_a,_l)            ctrl_pmem_recv(_a,_l)
#define gdb_pmem_send(_a,_l)            ctrl_pmem_send(_a,_l)
#define gdb_pmem_read(_a,_b,_l)         ctrl_pmem_read(_a,_b,_l)
#define gdb_pmem_write(_a,_b,_l)        ctrl_pmem_write(_a,_b,_l)

#define gdb_vmem_recv(_a,_l)            ctrl_vmem_recv(_a,_l)
#define gdb_vmem_send(_a,_l)            ctrl_vmem_send(_a,_l)
#define gdb_vmem_read(_a,_b,_l)         ctrl_vmem_read(_a,_b,_l)
#define gdb_vmem_write(_a,_b,_l)        ctrl_vmem_write(_a,_b,_l)

#ifdef __INIT__
void  gdb_init(mbi_t*);
#else
void  gdb_enable();
void  gdb_disable();
void  gdb_reset();
int   __gdb_preempt(uint8_t);
int   gdb_preempt(uint8_t);
void  gdbstub();
#endif

#endif
