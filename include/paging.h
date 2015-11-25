/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#ifndef __PAGING_H__
#define __PAGING_H__

#include <config.h>
#include <types.h>
#include <cr.h>
#include <pagemem.h>

#ifdef CONFIG_ARCH_AMD
#include <svm_vm.h>
#else
#include <vmx_vm.h>
#endif

/*
** Nested Paging mapping operators
*/
typedef enum
{
   NPG_OP_MAP = 0,
   NPG_OP_UNMAP,
   NPG_OP_REMAP,

} npg_op_type_t;

typedef void (*npg_op_fnc_t)(offset_t, uint64_t);

typedef struct npg_operator
{
   npg_op_fnc_t        fnc[3];
   size_t              sz, shf;
   struct npg_operator *nxt;

} __attribute__((packed)) npg_op_t;

/*
** Actions to be applied on mapping operation
*/
#define NPG_DO_ADDR   (1ULL<<0)
#define NPG_DO_OFFSET (1ULL<<1)
#define NPG_DO_PVL    (1ULL<<2)
#define NPG_DO_CACHE  (1ULL<<3)

typedef struct npg_configuration
{
   uint64_t mask;
   offset_t offset;

} __attribute__((packed)) npg_conf_t;

/*
** Nested Paging walk information
*/
#define NPG_WALK_TYPE_PML4E   0
#define NPG_WALK_TYPE_PDPE    1
#define NPG_WALK_TYPE_PDE     2
#define NPG_WALK_TYPE_PTE     3

typedef struct npg_walk_info
{
   offset_t addr;
   int      type;
   size_t   size;
   void     *entry;

} __attribute__((packed)) npg_wlk_t;

/*
** Controlling active nested paging
*/
#define NPG_DEFAULT  0
#define NPG_NR       1

#define npg_get_paging(_x)        (&info->vm.cpu.pg[(_x)])
#define npg_set_active_paging(_x) (info->vm.cpu.active_pg = (_x))
#define npg_get_active_paging()   (&info->vm.cpu.pg[info->vm.cpu.active_pg])

#define npg_set_active_paging_cpu()				\
   ({								\
      npg_cr3_set(npg_get_active_paging()->pml4);		\
      npg_set_asid(info->vm.cpu.active_pg+1);			\
   })

/*
** Nested Paging mapping functions
*/
npg_pte64_t* _npg_remap_finest_4K(offset_t);
npg_pte64_t* _npg_get_pte(offset_t);

void npg_map(offset_t, offset_t, uint64_t);
void npg_unmap(offset_t, offset_t);
void npg_setup_a20();

#ifndef __INIT__
/*
** Legacy && Nested walking functions
*/
struct vm_paging;

int  __pg_walk(cr3_reg_t*, offset_t, offset_t*, size_t*, int);
int  __npg_walk(struct vm_paging*, offset_t, npg_wlk_t*);

#define npg_walk(v,w)          __npg_walk(npg_get_active_paging(),v,w)

#endif

#endif
