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
#ifndef __VMM_H__
#define __VMM_H__

#include <config.h>
#include <types.h>
#include <segmem.h>
#include <pagemem.h>
#include <pool.h>
#include <dev.h>
#include <ctrl.h>
#include <gdbstub.h>
#include <slab.h>

/*
** General VMM settings
*/
#define VMM_MIN_STACK_SIZE               ((size_t)(2*PAGE_SIZE))
#define VMM_MIN_RAM_SZ                   ((size_t)(128<<20))

#define vmm_code_seg_idx                 1
#define vmm_data_seg_idx                 2
#define vmm_tss_seg_idx                  3

#define vmm_code_sel                     gdt_krn_seg_sel(vmm_code_seg_idx)
#define vmm_data_sel                     gdt_krn_seg_sel(vmm_data_seg_idx)
#define vmm_tss_sel                      gdt_krn_seg_sel(vmm_tss_seg_idx)

#define VMM_GDT_NR_DESC                  1+1+1+2*1 /* null:code:data:tss64 */

#define vmm_int32_desc(dsc,_isr_)        int32_desc(dsc,vmm_code_sel,_isr_)
#define vmm_int64_desc(dsc,_isr_)        int64_desc(dsc,vmm_code_sel,_isr_)

#define VMM_IDT_NR_DESC                  256
#define VMM_IDT_ISR_ALGN                  16

/*
** VMM data structures
*/
typedef struct vmm_paging
{
   /* strictly aligned */
   pml4e_t     *pml4;
   pdp_t       *pdp;
   pd64_t      *pd;

} __attribute__((packed)) vmm_pgmem_t;

typedef struct vmm_segmentation
{
   seg_desc_t   gdt[VMM_GDT_NR_DESC];   /* 8B aligned */
   int64_desc_t idt[VMM_IDT_NR_DESC];   /* 8B aligned */
   tss64_t      tss;

} __attribute__((packed)) vmm_sgmem_t;

typedef union vmm_cpu_skill
{
   struct
   {
      uint32_t  pg_1G:1;
      uint32_t  osxsave:1;
      uint32_t  paddr_sz:8;   /* max physical addr width */
      uint32_t  vaddr_sz:8;   /* max linear addr width */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmm_cpu_skill_t;

typedef struct vmm_cpu
{
   vmm_sgmem_t     *sg;      /* strictly aligned */
   vmm_pgmem_t     pg;
   vmm_cpu_skill_t skillz;
   uint64_t        max_paddr; /* maximum physical addr supported */
   uint64_t        max_vaddr; /* maximum linear addr supported */

} __attribute__((packed)) vmm_cpu_t;


#define VMM_IO_POOL_SZ  (4*PAGE_SIZE)

/*
** VMM structure
*/
typedef struct vmm
{
   vmm_cpu_t  cpu;              /* vmm cpu info */
   vmm_pool_t pool;             /* vmm page pool */
   offset_t   io_pool;          /* vmm io cache pool */
   vmm_slab_t slab;             /* vmm slab allocator caches */
   vmm_ctrl_t ctrl;             /* vmm controller */
   offset_t   entry;            /* vmm entry point */
   offset_t   base;             /* vmm relocation addr */
   size_t     size;             /* vmm loaded elf size */
   offset_t   stack_bottom;     /* vmm stack bottom location */
#ifdef CONFIG_GDBSTUB
   gdbstub_t  gstub;            /* vmm GDB stub */
#endif
} __attribute__((packed)) vmm_t;

/*
** Functions
*/
#ifdef __INIT__

void vmm_init();
void vmm_start() __regparm__(1);

#ifdef CONFIG_ARCH_AMD
#include <svm_vmm.h>
#define vmm_vmc_init()   svm_vmm_init()
#define vmm_vmc_start()  svm_vmm_start(info->vm.cpu.gpr,info->vmm.entry)
#else
#include <vmx_vmm.h>
#define vmm_vmc_init()   vmx_vmm_init()
#define vmm_vmc_start()  vmx_vmm_start(info->vm.cpu.gpr)
#endif

#endif

#endif
