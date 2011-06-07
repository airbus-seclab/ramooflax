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

/*
** MTRR related MSRs
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

#define IA32_MTRR_DEF_TYPE           0x2ff

typedef union ia32_mtrr_default_type
{
   raw_msr_entry_t;

   struct
   {
      uint64_t  type:8;
      uint64_t  rsv1:2;
      uint64_t  fe:1;
      uint64_t  e:1;

   } __attribute__((packed));

} __attribute__((packed)) ia32_mtrr_def_type_t;

#endif
