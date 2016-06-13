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
#ifndef __DEV_PIC_H__
#define __DEV_PIC_H__

#include <types.h>
#include <pic.h>
#include <io.h>

typedef struct programmable_interrupt_controler
{
   uint8_t      base;        /* I/O port */
   uint8_t      wait_icw;    /* ICW waited */

   pic_icw1_t   icw1;
   pic_icw2_t   icw2;
   pic_icw3_t   icw3;
   pic_icw4_t   icw4;

   pic_ocw2_t   ocw2;
   pic_ocw3_t   ocw3;

   pic_ocw1_t   imr;         /* irq lines which are masked        */
   uint8_t      irr;         /* irq lines requesting an interrupt */
   uint8_t      isr;         /* irq lines that are being serviced */
   uint8_t      poll;        /* poll mode data read */

} pic_t;

/*
** Functions
*/
#ifdef __INIT__
#define dev_pic_init(_pic_, n)                          \
   ({                                                   \
      (_pic_)->base     = PIC##n;                       \
      (_pic_)->icw2.raw = DFLT_PIC##n##_ICW2;           \
      (_pic_)->wait_icw = 1;                            \
      (_pic_)->imr.raw  = in(PIC_IMR((_pic_)->base));   \
   })

#else
int     dev_pic(pic_t*, io_insn_t*);
int     dev_pic_acting(uint8_t, uint32_t*);

uint8_t pic_irr(uint8_t);
uint8_t pic_isr(uint8_t);

int     __dev_pic_status(pic_t*, io_insn_t*);
int     __dev_pic_imr(pic_t*, uint8_t);
int     __dev_pic_ocw2(pic_t*, uint8_t);
int     __dev_pic_ocw3(pic_t*, uint8_t);

int     __dev_pic_icw1(pic_t*, uint8_t);
int     __dev_pic_icw2(pic_t*, uint8_t);
int     __dev_pic_icw3(pic_t*, uint8_t);
int     __dev_pic_icw4(pic_t*, uint8_t);
#endif

#endif

