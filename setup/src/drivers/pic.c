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
#include <pic.h>
#include <excp.h>
#include <info_data.h>

extern info_data_t *info;

void pic_init()
{
   pic_icw1_t   icw1;
   pic_icw2_t   icw2;
   pic_icw3_t   icw3;
   pic_icw4_t   icw4;

   /*
   ** ICW1 (bit4=1):
   **  - need icw4
   **  - cascaded mode
   */
   icw1.raw = 0x11;
   out(icw1.raw, PIC_ICW1(PIC1));
   out(icw1.raw, PIC_ICW1(PIC2));

   /*
   ** ICW2:
   **  - remap IRQ[00-07] to IDT[32-39]
   **  - remap IRQ[08-15] to IDT[40-47]
   */
   icw2.raw = NR_EXCP;
   out(icw2.raw, PIC_ICW2(PIC1));

   icw2.raw = NR_EXCP+8;
   out(icw2.raw, PIC_ICW2(PIC2));

   /*
   ** ICW3:
   **  - master set irq mask choosing irq pin to cascade to the slave
   **  - slave stores master irq "number" used to cascade
   */
   icw3.raw = 0;
   icw3.master.irq2 = 1;
   out(icw3.raw, PIC_ICW3(PIC1));

   icw3.raw = 0;
   icw3.slave.id = 2;
   out(icw3.raw, PIC_ICW3(PIC2));

   /*
   ** ICW4:
   **  - x86 mode
   **  - normal EOI
   **  - non buffered
   **  - not special fully nested
   */
   icw4.raw = 1;
   out(icw4.raw, PIC_ICW4(PIC1));
   out(icw4.raw, PIC_ICW4(PIC2));
}

