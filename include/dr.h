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
#ifndef __DR_H__
#define __DR_H__

#include <types.h>

#define DR6_BD_BIT    13
#define DR6_BD        (1<<DR6_BD_BIT)

/*
** DR6 debug register
*/
typedef union debug_register_6
{
   struct
   {
      uint64_t    b0:1;
      uint64_t    b1:1;
      uint64_t    b2:1;
      uint64_t    b3:1;
      uint64_t    fixed:9;
      uint64_t    bd:1;
      uint64_t    bs:1;
      uint64_t    bt:1;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) dr6_reg_t;

/*
** DR7 debug register
*/
#define DR7_COND_X     0
#define DR7_COND_W     1
#define DR7_COND_IO    2 /* unused */
#define DR7_COND_RW    3
#define DR7_COND_SS    4 /* internal */

#define DR7_LEN_1      0
#define DR7_LEN_2      1
#define DR7_LEN_8      2
#define DR7_LEN_4      3

#define DR7_GD_BIT    13
#define DR7_GD        (1<<DR7_GD_BIT)

typedef union debug_register_7
{
   struct
   {
      uint64_t    l0:1;
      uint64_t    g0:1;
      uint64_t    l1:1;
      uint64_t    g1:1;
      uint64_t    l2:1;
      uint64_t    g2:1;
      uint64_t    l3:1;
      uint64_t    g3:1;
      uint64_t    le:1;
      uint64_t    ge:1;
      uint64_t    r1:3;   /* reserved */
      uint64_t    gd:1;
      uint64_t    r2:2;   /* reserved */
      uint64_t    rw0:2;
      uint64_t    len0:2;
      uint64_t    rw1:2;
      uint64_t    len1:2;
      uint64_t    rw2:2;
      uint64_t    len2:2;
      uint64_t    rw3:2;
      uint64_t    len3:2;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) dr7_reg_t;

/*
** Mov to/from dr are exclusive:
**  - if 64 bits -> get 64 bits
**  - if 32 bits -> get 32 bits
*/
#define __get_dr(_n)     ({offset_t x;asm volatile("mov %%dr"#_n", %0":"=r"(x));x;})
#define __set_dr(_n,_x)  asm volatile("mov %0, %%dr"#_n ::"r"(_x))

#define get_dr0()        __get_dr(0)
#define get_dr1()        __get_dr(1)
#define get_dr2()        __get_dr(2)
#define get_dr3()        __get_dr(3)
#define get_dr6()        __get_dr(6)
#define get_dr7()        __get_dr(7)

#define set_dr0(x)       __set_dr(0,x)
#define set_dr1(x)       __set_dr(1,x)
#define set_dr2(x)       __set_dr(2,x)
#define set_dr3(x)       __set_dr(3,x)
#define set_dr6(x)       __set_dr(6,x)
#define set_dr7(x)       __set_dr(7,x)

/*
** Functions
*/
#ifndef __INIT__

#define DR_FAIL       0
#define DR_FAULT      1
#define DR_SUCCESS    2

#define __valid_dr_regs(_gpr,_dr)     (_gpr <= GPR64_RAX && _dr <= 7)
#define __valid_dr_access()           (__rmode() || !__cpl)

int     __resolve_dr(uint8_t, uint8_t, uint8_t);

#endif

offset_t get_dr(uint8_t);
void     set_dr(uint8_t, offset_t);

#endif
