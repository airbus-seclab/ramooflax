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
#ifndef __APIC_H__
#define __APIC_H__

#include <types.h>

/*
** IMCR
*/
#define IMCR_ADDR     0x22
#define IMCR_DATA     0x23

#define IMCR_REG      0x70

#define __imcr_set(_x_)                         \
   ({                                           \
      out(IMCR_REG, IMCR_ADDR);                 \
      out((_x_), IMCR_DATA);                    \
   })

#define imcr_set_pic()     __imcr_set(0)
#define imcr_set_apic()    __imcr_set(1)

/*
** local apic register type: 128 bits long
*/
typedef struct local_apic_register
{
   raw64_t  low;
   raw64_t  high;

} __attribute__((packed)) lapic_reg_t;

/*
** Local APIC register address map
**
** (4KB aligned and length, require strong uncachable mem)
*/
typedef struct local_apic
{
   lapic_reg_t    rsrvd0;
   lapic_reg_t    rsrvd1;
   lapic_reg_t    id;
   lapic_reg_t    version;
   lapic_reg_t    rsrvd2;
   lapic_reg_t    rsrvd3;
   lapic_reg_t    rsrvd4;
   lapic_reg_t    rsrvd5;
   lapic_reg_t    tpr;
   lapic_reg_t    apr;
   lapic_reg_t    ppr;
   lapic_reg_t    eoi;
   lapic_reg_t    rsrvd6;
   lapic_reg_t    dest;
   lapic_reg_t    dest_format;
   lapic_reg_t    spurious_irq;
   lapic_reg_t    isr[8];
   lapic_reg_t    tmr[8];
   lapic_reg_t    irr[8];
   lapic_reg_t    error;
   lapic_reg_t    rsrvd7[7];
   lapic_reg_t    icr_low;    /* icr[00-31] */
   lapic_reg_t    icr_mid;    /* icr[32-63] */
   lapic_reg_t    lvt_timer;
   lapic_reg_t    lvt_sens;
   lapic_reg_t    lvt_perf;
   lapic_reg_t    lvt_lint0;
   lapic_reg_t    lvt_lint1;
   lapic_reg_t    lvt_error;
   lapic_reg_t    timer_init_count;
   lapic_reg_t    timer_crt_count;
   lapic_reg_t    rsrvd8[4];
   lapic_reg_t    timer_divide;
   lapic_reg_t    rsrvd9;

} __attribute__((packed)) lapic_t;



#endif
