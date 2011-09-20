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
#ifndef __GDB_STUB_H__
#define __GDB_STUB_H__

#include <types.h>
#include <gpr.h>
#include <ctrl_hdl.h>
#include <gdb_core.h>
#include <gdb_vmm.h>
#include <gdb_brk.h>
#include <gdb_sstep.h>

#define GDB_PKT_SZ              4
#define GDB_ACKPKT_SZ           (GDB_PKT_SZ+1)

#define GDB_ANSWER_SZ           1024
#define GDB_BUF_SZ              (GDB_ANSWER_SZ - GDB_ACKPKT_SZ)

#define GDB_PKT_BYTE           '$'
#define GDB_END_BYTE           '#'
#define GDB_ACK_BYTE           '+'
#define GDB_NAK_BYTE           '-'
#define GDB_NTF_BYTE           '%'
#define GDB_RLN_BYTE           '*'
#define GDB_ESC_BYTE           '}'
#define GDB_ESC_XOR            ' '

/* default interrupt sequence CTL+C */
#define GDB_INT_BYTE            3

#define GDB_CMD_THREAD         'H'
#define GDB_CMD_STOP_REASON    '?'
#define GDB_CMD_R_GPR          'g'
#define GDB_CMD_W_GPR          'G'
#define GDB_CMD_R_REG          'p'
#define GDB_CMD_W_REG          'P'
#define GDB_CMD_R_MEM          'm'
#define GDB_CMD_W_MEM          'M'
#define GDB_CMD_CONT           'c'
#define GDB_CMD_STEP           's'
#define GDB_CMD_KILL           'k'
#define GDB_CMD_DETACH         'D'

#define GDB_CMD_S_BRK          'Z'
#define GDB_CMD_R_BRK          'z'

#define GDB_BRK_TYPE_MEM        0
#define GDB_BRK_TYPE_HRD_X      1
#define GDB_BRK_TYPE_HRD_W      2
#define GDB_BRK_TYPE_HRD_RW     4

#define GDB_CMD_QUERY          'q'
#define GDB_QUERY_THREAD       'C'

/*
** GDB stub Data structures
*/
#define GDB_OS_AFFINITY_UNKNOWN    0
#define GDB_OS_AFFINITY_LINUX26    1
#define GDB_OS_AFFINITY_WIN7       2
#define GDB_OS_AFFINITY_WINXP      3

typedef union vmm_ctrl_dbg_status
{
   struct
   {
      uint8_t    on:1;         /* debugger is active */
      uint8_t    vm_hlt:1;     /* vm is stopped */
      uint8_t    acked:1;      /* gdb ack sent */
      uint8_t    cr3:1;        /* gdb acting on specific cr3 */
      uint8_t    keep_cr3:1;   /* remember specific cr3 over sessions */
      uint8_t    dr6:1;        /* must clean dr6 */
      uint8_t    traps:1;      /* traps status */
      uint8_t    utraps:1;     /* traps update */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) vmm_ctrl_dbg_sts_t;

#define gdb_traps_configured()  (info->vmm.ctrl.dbg.status.traps)
#define gdb_set_traps(_x)			\
   ({						\
      info->vmm.ctrl.dbg.status.traps  = (_x);	\
      info->vmm.ctrl.dbg.status.utraps = 1;	\
   })
#define gdb_traps_need_update() (info->vmm.ctrl.dbg.status.utraps)
#define gdb_set_traps_updated() (info->vmm.ctrl.dbg.status.utraps = 0)

typedef struct vmm_ctrl_debugger
{
   vmm_ctrl_dbg_sts_t  status;      /* stub status */
   uint8_t             affinity;    /* os affinity */
   uint8_t             last_reason; /* last stop reason */

   vmm_ctrl_dbg_brk_t  brk;
   vmm_ctrl_dbg_stp_t  stp;
   uint32_t            excp;

   cr3_reg_t           stored_cr3;  /* recv cache */
   cr3_reg_t           *active_cr3; /* tracked cr3 */

} __attribute__((packed)) vmm_ctrl_dbg_t;

/*
** Functions
*/
#ifndef __INIT__

static inline uint8_t* gdb_escape(uint8_t *data, uint8_t byte)
{
   *data++ = GDB_ESC_BYTE;
   *data++ = byte ^ GDB_ESC_XOR;

   return data;
}

#define gdb_ok()           ({ctrl_write((uint8_t*)"+$OK#9a",  7);})
#define gdb_unsupported()  ({ctrl_write((uint8_t*)"+$#00",    5);})
#define gdb_err_gen()      ({ctrl_write((uint8_t*)"+$E00#a5", 8);})
#define gdb_err_mem()      ({ctrl_write((uint8_t*)"+$E0e#da", 8);})
#define gdb_err_oom()      ({ctrl_write((uint8_t*)"+$E0c#d8", 8);})
#define gdb_err_inv()      ({ctrl_write((uint8_t*)"+$E16#ac", 8);})

#define gdb_ack()				\
   ({						\
      uint8_t x = GDB_ACK_BYTE;			\
      ctrl_write(&x,1);				\
      info->vmm.ctrl.dbg.status.acked = 1;	\
   })

#define gdb_force_acked()			\
   ({						\
      if(!info->vmm.ctrl.dbg.status.acked)	\
	 info->vmm.ctrl.dbg.status.acked = 1;	\
   })

#define gdb_nak()			\
   ({					\
      uint8_t x = GDB_NAK_BYTE;		\
      ctrl_write(&x,1);			\
   })

#define gdb_wait_ack()						\
   ({								\
      uint8_t x;						\
      while(ctrl_read(&x, 1) != 1 && x != GDB_ACK_BYTE);	\
   })

#define gdb_set_last_stop_reason(_x)		\
   ({						\
      info->vmm.ctrl.dbg.last_reason = (_x);	\
   })

#define gdb_last_stop_reason()	      (info->vmm.ctrl.dbg.last_reason)
#define gdb_set_enable(_x)            (info->vmm.ctrl.dbg.status.on = (_x))
#define gdb_enabled()                 (info->vmm.ctrl.dbg.status.on)
#define gdb_disabled()                (!gdb_enabled())

#define gdb_preempt(_n_)			\
   ({						\
      gdb_send_stop_reason(_n_);		\
      gdb_set_last_stop_reason(_n_);		\
      ctrl_set_active();			\
      1;					\
   })

#define gdb_set_affinity(_a_)    (info->vmm.ctrl.dbg.affinity = (_a_))
#define gdb_get_affinity()       (info->vmm.ctrl.dbg.affinity)

/*
** gdb handlers decision
*/
#define GDB_FAIL             0
#define GDB_PREEMPT          1
#define GDB_RELEASE          2
#define GDB_IGNORE           3
#define GDB_FAULT            4

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

/*
** Prototypes
*/
void   gdb_add_str(char*, size_t);
void   gdb_add_byte(uint8_t);
void   gdb_add_number(uint64_t, size_t, uint8_t);
int    gdb_get_byte(uint8_t*, size_t, uint8_t*);
int    gdb_get_number(uint8_t*, size_t, uint64_t*, uint8_t);

void   gdb_send_packet();
void   gdb_send_stop_reason(uint8_t);

int    gdb_read_mem(offset_t, uint8_t*, size_t);
int    gdb_write_mem(offset_t, uint8_t*, size_t);

int    __gdb_setup_reg_op(uint8_t*,size_t,raw64_t**,size_t*,raw64_t*,uint8_t,uint8_t);

size_t gdb_stub(uint8_t*, size_t);
void   gdb_stub_post();

#endif

#endif
