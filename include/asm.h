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
#ifndef __ASM_H__
#define __ASM_H__

#include <gpr.h>
#include <cr.h>
#include <dr.h>
#include <msr.h>
#include <mtrr.h>
#include <insn.h>
#include <cpuid.h>

#include <config.h>
#ifdef CONFIG_ARCH_AMD
#include <amd.h>
#else
#include <intel.h>
#endif

/*
** Various assembly routines
*/
#define _2_power(pow)     (1<<(pow))

/*
** Binary-Coded-Decimal (BCD)
**
** Decimal:     43           52
** BCD:      0100 0011    0101 0010
**
** each nibble (4 bits part) can not be greater than 1001 (decimal 9)
*/
#define bin2bcd(_vAl_)    ((((_vAl_)&0xff)/10)<<4 | (_vAl_)%10)
#define bcd2bin(_VaL_)    ((((_VaL_)>>4)&0xf)*10 + ((_VaL_)&0xf))

/*
** Wait "_n_" microsecond(s)
*/
#define io_wait(_n_)      asm volatile("1: outb %%al, $0x80 ; loop 1b"::"c"(_n_))

/*
** Inhibite/Restore interrupt flag
*/
#define atomic_on()                  force_interrupts_off()
#define atomic_off()                 force_interrupts_on()
#define interrupts_enabled()         (get_flags() & EFLAGS_IF)
#define disable_interrupts(flags)    ({save_flags(flags);atomic_on();})
#define enable_interrupts(flags)     ({save_flags(flags);atomic_off();})
#define restore_interrupts(flags)    load_flags(flags)

#endif
