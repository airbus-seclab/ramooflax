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
#ifndef __MTRR_H__
#define __MTRR_H__

#include <types.h>
#include <msr.h>

/*
** MTRR memory types (decreasing precedence)
*/
#define MTRR_MEM_TYPE_UC             0
#define MTRR_MEM_TYPE_WC             1
#define MTRR_MEM_TYPE_WT             4
#define MTRR_MEM_TYPE_WP             5
#define MTRR_MEM_TYPE_WB             6

/*
** MTRR capabilities
*/
#define IA32_MTRRCAP                 0xfe

typedef union ia32_mtrr_capabilites
{
   struct
   {
      uint64_t  vcnt:8;
      uint64_t  fix:1;
      uint64_t  r0:1;
      uint64_t  wc:1;
      uint64_t  smrr:1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) ia32_mtrr_cap_t;

#define rd_msr_ia32_mtrr_cap(_m)   rd_msr64(IA32_MTRRCAP, (_m).edx, (_m).eax)

/*
** MTRR default type
*/
#define IA32_MTRR_DEF_TYPE           0x2ff

typedef union ia32_mtrr_default_type
{
   struct
   {
      uint64_t  type:8;
      uint64_t  r0:2;
      uint64_t  fe:1;
      uint64_t  e:1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) ia32_mtrr_def_t;

#define rd_msr_ia32_mtrr_def(_m)   rd_msr64(IA32_MTRR_DEF_TYPE, (_m).edx, (_m).eax)
#define wr_msr_ia32_mtrr_def(_m)   wr_msr64(IA32_MTRR_DEF_TYPE, (_m).edx, (_m).eax)

#define __mtrr_def_set_enable(_x)	\
   ({					\
      ia32_mtrr_def_t def;		\
      rd_msr_ia32_mtrr_def(def);	\
      def.e = _x;			\
      wr_msr_ia32_mtrr_def(def);	\
   })

#define disable_mtrr()    __mtrr_def_set_enable(0)
#define enable_mtrr()     __mtrr_def_set_enable(1)

/*
** MTRR variable range
*/
typedef union ia32_mtrr_physbase
{
   struct
   {
      uint64_t   type:8;
      uint64_t   r0:4;
      uint64_t   base:52;

   } __attribute__((packed));

      msr_t;

} __attribute__((packed)) ia32_mtrr_physbase_t;

typedef union ia32_mtrr_physmask
{
   struct
   {
      uint64_t   r0:11;
      uint64_t   v:1;
      uint64_t   mask:52; /* range if B & M == @ & M */

   } __attribute__((packed));

      msr_t;

} __attribute__((packed)) ia32_mtrr_physmask_t;

/*
** MTRR variable range
*/
#define IA32_MTRR_PHYSBASE0          0x200
#define IA32_MTRR_PHYSMASK0          0x201
#define IA32_MTRR_PHYSBASE1          0x202
#define IA32_MTRR_PHYSMASK1          0x203
#define IA32_MTRR_PHYSBASE2          0x204
#define IA32_MTRR_PHYSMASK2          0x205
#define IA32_MTRR_PHYSBASE3          0x206
#define IA32_MTRR_PHYSMASK3          0x207
#define IA32_MTRR_PHYSBASE4          0x208
#define IA32_MTRR_PHYSMASK4          0x209
#define IA32_MTRR_PHYSBASE5          0x20a
#define IA32_MTRR_PHYSMASK5          0x20b
#define IA32_MTRR_PHYSBASE6          0x20c
#define IA32_MTRR_PHYSMASK6          0x20d
#define IA32_MTRR_PHYSBASE7          0x20e
#define IA32_MTRR_PHYSMASK7          0x20f
#define IA32_MTRR_PHYSBASE8          0x210
#define IA32_MTRR_PHYSMASK8          0x211
#define IA32_MTRR_PHYSBASE9          0x212
#define IA32_MTRR_PHYSMASK9          0x213

#define rd_msr_ia32_mtrr_physbase(_m,_n)  rd_msr64(0x200+((_n)*2),   (_m).edx, (_m).eax)
#define rd_msr_ia32_mtrr_physmask(_m,_n)  rd_msr64(0x200+((_n)*2)+1, (_m).edx, (_m).eax)

#define wr_msr_ia32_mtrr_physbase(_m,_n)  wr_msr64(0x200+((_n)*2),   (_m).edx, (_m).eax)
#define wr_msr_ia32_mtrr_physmask(_m,_n)  wr_msr64(0x200+((_n)*2)+1, (_m).edx, (_m).eax)

/*
** MTRR fixed range
** . define memory < 1MB
** . each register holds type for 8 blocks
** . take prority over variable if enabled (mtrr_def.fe)
*/

/* from 0 to 0x7ffff: 8 blocks each 64KB */
#define IA32_MTRR_FIX64K_00000       0x250

/* from 0x80000 to 0xbffff: 16 blocks each 16KB */
#define IA32_MTRR_FIX16K_80000       0x258
#define IA32_MTRR_FIX16K_a0000       0x259

/* from 0xc0000 to 0xfffff: 64 blocks each 4KB */
#define IA32_MTRR_FIX4K_c0000        0x268
#define IA32_MTRR_FIX4K_c8000        0x269
#define IA32_MTRR_FIX4K_d0000        0x26a
#define IA32_MTRR_FIX4K_d8000        0x26b
#define IA32_MTRR_FIX4K_e0000        0x26c
#define IA32_MTRR_FIX4K_e8000        0x26d
#define IA32_MTRR_FIX4K_f0000        0x26e
#define IA32_MTRR_FIX4K_f8000        0x26f

#endif
