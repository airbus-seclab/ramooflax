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
#ifndef __DBG_HARD_BRK_H__
#define __DBG_HARD_BRK_H__

#include <types.h>
#include <dr.h>
#include <ctrl_evt.h>

/*
** Hardware breakpoints
*/
typedef union dbg_hardware_breakpoints_status
{
   struct
   {
      uint8_t    insn:1;    /* insn hard breakpoints installed */
      uint8_t    on:1;      /*      hard breakpoints installed */
      uint8_t    dis:1;     /*      hard breakpoints disarmed  */
      uint8_t    rf:1;      /* saved vm rflags.rf */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) dbg_hard_brk_sts_t;

#define DBG_HARD_BRK_NR  4

typedef struct dbg_hardware_breakpoints
{
   dbg_hard_brk_sts_t sts;
   dr7_reg_t          dr7;
   ctrl_evt_hdl_t     hdlr[DBG_HARD_BRK_NR]; /* specific breakpoint handlers */

} __attribute__((packed)) dbg_hard_brk_t;

/*
** Functions
*/
#define ___hbrk                    (info->vmm.ctrl.dbg.hard.brk)
#define ___hbrk_enbl_idx(_n)       ((_n)*2+1)
#define ___hbrk_conf_idx(_n)       (16+(_n)*4)

#define __hbrk_type_of(_c)         ((_c)&3)
#define __hbrk_len_of(_c)          (((_c)>>2)&3)

#define __hbrk_set_enbl(_n)        (1<<___hbrk_enbl_idx(_n))
#define __hbrk_set_conf(_n,_c)     ((_c)<<___hbrk_conf_idx(_n))
#define __hbrk_mke_conf(_t,_l)     (((((_l)&3)<<2)|((_t)&3)) & 0xf)

#define __hbrk_setup_bp(_n,_t,_l)                                       \
   (___hbrk.dr7.low |= __hbrk_set_conf(_n,__hbrk_mke_conf(_t,_l))|__hbrk_set_enbl(_n))

#define __hbrk_delete_bp(_n)                                            \
   (___hbrk.dr7.low &= ~(__hbrk_set_conf(_n,0xf)|(0x3<<___hbrk_enbl_idx(_n))))

#define __hbrk_get_config(_n)      ((___hbrk.dr7.low>>___hbrk_conf_idx(_n)) & 0xf)
#define __hbrk_get_type(_n)        ((___hbrk.dr7.low>>___hbrk_conf_idx(_n)) & 0x3)
#define __hbrk_enabled(_n)         (___hbrk.dr7.blow & __hbrk_set_enbl((_n)))
#define __hbrk_raised(_n)          (__dr6.blow & (1<<(_n)))

#define dbg_hard_brk_dr7_clean()        \
   ({                                   \
      ___hbrk.dr7.low &= 0x100;         \
      __dr7.low &= 0x100;               \
      __post_access(__dr7);             \
   })

#define dbg_hard_brk_enabled()             (___hbrk.sts.on)
#define dbg_hard_brk_insn_enabled()        (___hbrk.sts.insn)
#define dbg_hard_brk_saved_rf()            (___hbrk.sts.rf)
#define dbg_hard_brk_disarmed()            (___hbrk.sts.dis)

#define dbg_hard_brk_set_enable(_x)        (___hbrk.sts.on=(_x))
#define dbg_hard_brk_set_disarm(_x)        (___hbrk.sts.dis=(_x))
#define dbg_hard_brk_insn_set_enable(_x)   (___hbrk.sts.insn=(_x))
#define dbg_hard_brk_save_rf(_x)           (___hbrk.sts.rf=(_x))
#define dbg_hard_brk_set_hdlr(_n,_h)       (___hbrk.hdlr[(_n)]=(_h))
#define dbg_hard_brk_get_hdlr(_n)          (___hbrk.hdlr[(_n)])


void dbg_hard_brk_enable();
void dbg_hard_brk_disable();
void dbg_hard_brk_reset();

void dbg_hard_brk_disarm();
void dbg_hard_brk_rearm();

int  dbg_hard_brk_set(offset_t, uint8_t, uint8_t, ctrl_evt_hdl_t);
int  dbg_hard_brk_del(offset_t, uint8_t, uint8_t);
int  dbg_hard_brk_resume();
int  dbg_hard_brk_event(ctrl_evt_hdl_t*);

#endif
