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
#ifndef __CTRL_H__
#define __CTRL_H__

#include <types.h>
#include <ctrl_evt.h>
#include <dbg.h>

typedef union controller_status
{
   struct
   {
      uint8_t    cr3:1;        /* acting on specific cr3 */
      uint8_t    keep:1;       /* keep specific cr3 upon sessions */
      uint8_t    traps:1;      /* traps status */
      uint8_t    utraps:1;     /* traps update */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) ctrl_sts_t;

#define ctrl_cr3_enabled()         (info->vmm.ctrl.status.cr3)
#define ctrl_cr3_keep()            (info->vmm.ctrl.status.keep)
#define ctrl_traps_enabled()       (info->vmm.ctrl.status.traps)
#define ctrl_traps_updated()       (info->vmm.ctrl.status.utraps)

#define ctrl_set_cr3(_x)           (info->vmm.ctrl.status.cr3=(_x))
#define ctrl_set_cr3_keep(_x)      (info->vmm.ctrl.status.keep=(_x))
#define ctrl_set_traps(_x)         (info->vmm.ctrl.status.traps=(_x))
#define ctrl_traps_set_update(_x)  (info->vmm.ctrl.status.utraps=(_x))

#define CTRL_FILTER_NPF     (1<<0)
#define CTRL_FILTER_HYP     (1<<1)
#define CTRL_FILTER_CPUID   (1<<2)

#define CTRL_CPUID_ALL      (-1U)

typedef struct controller_user
{
   uint32_t    excp;   /* user requested excp     intercepts */
   uint32_t    cr_rd;  /* user requested read  CR intercepts */
   uint32_t    cr_wr;  /* user requested write CR intercepts */
   uint64_t    filter; /* user requested various  intercepts */
   uint32_t    cpuid;  /* user cpuid index to filter on */

} __attribute__((packed)) ctrl_usr_t;

#define __ctrl_set_cr_mask(_cr,_m) (_cr = _m & 0x1ff)  /* until cr8 */

#define ctrl_usr_set_cr_rd(_m)				\
   ({							\
      __ctrl_set_cr_mask(info->vmm.ctrl.usr.cr_rd, _m);	\
      __update_cr_read_mask();				\
   })

#define ctrl_usr_set_cr_wr(_m)				\
   ({							\
      __ctrl_set_cr_mask(info->vmm.ctrl.usr.cr_wr, _m);	\
      __update_cr_write_mask();				\
   })

#define CTRL_OS_AFFINITY_UNKNOWN    0
#define CTRL_OS_AFFINITY_LINUX26    1
#define CTRL_OS_AFFINITY_WIN7       2
#define CTRL_OS_AFFINITY_WINXP      3

typedef struct vmm_controller
{
   ctrl_sts_t  status;
   uint8_t     affinity;

   cr3_reg_t   stored_cr3;
   cr3_reg_t   *active_cr3;

   ctrl_evt_t  event;
   ctrl_usr_t  usr;
   ctrl_dbg_t  dbg;

   raw64_t     vmexit_cnt;

} __attribute__((packed)) vmm_ctrl_t;

#define ctrl_affinity()            (info->vmm.ctrl.affinity)
#define ctrl_set_affinity(_a_)     (info->vmm.ctrl.affinity=(_a_))

/*
** Functions
*/
int  controller();
void ctrl_active_cr3_enable(cr3_reg_t);
void ctrl_active_cr3_disable();
void ctrl_active_cr3_reset();
void ctrl_usr_reset();

int  __ctrl_active_cr3_check(int);

#define ctrl_active_cr3_check()   __ctrl_active_cr3_check(3)

#define ctrl_mem_read(_a,_d,_l)   __vm_read_mem(info->vmm.ctrl.active_cr3,_a,_d,_l)
#define ctrl_mem_write(_a,_d,_l)  __vm_write_mem(info->vmm.ctrl.active_cr3,_a,_d,_l)

#define ctrl_mem_read_with(_c,_a,_d,_l)   __vm_read_mem(_c,_a,_d,_l)
#define ctrl_mem_write_with(_c,_a,_d,_l)  __vm_write_mem(_c,_a,_d,_l)

#define ctrl_mem_send(_a,_l)      __vm_send_mem(info->vmm.ctrl.active_cr3,_a,_l)
#define ctrl_mem_recv(_a,_l)      __vm_recv_mem(info->vmm.ctrl.active_cr3,_a,_l)

#endif
