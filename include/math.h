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
#ifndef __MATH_H__
#define __MATH_H__

#include <types.h>

static inline uint32_t abs(int x)
{
   return (x >= 0) ? x : -x;
}

/* precondition:  a > b */
static inline uint32_t pgcd(uint32_t a, uint32_t b)
{
   return b ? pgcd(b, a%b) : a;
}

#define max(a,b)  ((a)>(b)?(a):(b))
#define min(a,b)  ((a)<(b)?(a):(b))

#ifdef __X86_64__
#define __divrm(a,b,r,m)                                                \
   asm volatile ("div %%rcx":"=a"(r),"=d"(m):"a"(a),"d"(0),"c"(b))
#else
#define __divrm(a,b,r,m)                                                \
   asm volatile ("div %%ecx":"=a"(r),"=d"(m):"a"(a),"d"(0),"c"(b))
#endif

#define adc16(a,b)                                                      \
   ({                                                                   \
      uint16_t v;                                                       \
      asm volatile (                                                    \
         "add %%dx, %%ax\n"                                             \
         "adc $0, %%ax"                                                 \
         :"=a"(v)                                                       \
         :"a"((a)),"d"((b))                                             \
         :"memory");                                                    \
      v;                                                                \
   })

#endif
