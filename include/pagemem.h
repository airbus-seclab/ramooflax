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
#ifndef __PAGEMEM_H__
#define __PAGEMEM_H__

#include <types.h>

/**************************************************************************
 *************************** Common definitions ***************************
 **************************************************************************/

/*
** Paging bits accessing
*/
#define PG_P_BIT        0
#define PG_RW_BIT       1
#define PG_LVL_BIT      2
#define PG_PWT_BIT      3
#define PG_PCD_BIT      4
#define PG_ACC_BIT      5
#define PG_DRT_BIT      6
#define PG_PAT_BIT      7
#define PG_GLB_BIT      8

#define PG_P            (1<<PG_P_BIT)
#define PG_RO           0
#define PG_RW           (1<<PG_RW_BIT)
#define PG_KRN          0
#define PG_USR          (1<<PG_LVL_BIT)
#define PG_PWT          (1<<PG_PWT_BIT)
#define PG_PCD          (1<<PG_PCD_BIT)
#define PG_ACC          (1<<PG_ACC_BIT)
#define PG_DRT          (1<<PG_DRT_BIT)
#define PG_PAT          (1<<PG_PAT_BIT)
#define PG_GLB          (1<<PG_GLB_BIT)

/*
** Large pages
*/
#define PG_PS_BIT       7
#define PG_LPAT_BIT     12

#define PG_PS           (1<<PG_PS_BIT)
#define PG_LPAT         (1<<PG_LPAT_BIT)

/*
** Long mode
*/
#define PG_NX_BIT       63
#define PG_NX           (1UL<<PG_NX_BIT)

/************************************************************************
 ********************** 32 bits paging without PAE **********************
 ************************************************************************/

typedef union page_directory_entry
{
   struct
   {
      uint32_t  p:1;
      uint32_t  rw:1;
      uint32_t  lvl:1;
      uint32_t  pwt:1;
      uint32_t  pcd:1;
      uint32_t  acc:1;
      uint32_t  mbz:3;
      uint32_t  avl:3;
      uint32_t  addr:20;

   } __attribute__((packed));

   struct
   {
      uint32_t  p:1;
      uint32_t  rw:1;
      uint32_t  lvl:1;
      uint32_t  pwt:1;
      uint32_t  pcd:1;
      uint32_t  acc:1;
      uint32_t  d:1;
      uint32_t  ps:1;
      uint32_t  g:1;
      uint32_t  avl:3;
      uint32_t  pat:1;
      uint32_t  rsv:9;
      uint32_t  addr:10;

   } __attribute__((packed)) page;

   raw32_t;

} __attribute__((packed)) pde32_t;

typedef union page_table_entry
{
   struct
   {
      uint32_t  p:1;
      uint32_t  rw:1;
      uint32_t  lvl:1;
      uint32_t  pwt:1;
      uint32_t  pcd:1;
      uint32_t  acc:1;
      uint32_t  d:1;
      uint32_t  pat:1;
      uint32_t  g:1;
      uint32_t  avl:3;
      uint32_t  addr:20;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pte32_t;

/*
** 32 bits paging usefull macros
*/
#define PDE32_PER_PD                 1024UL
#define PTE32_PER_PT                 1024UL

#define PG_4K_SHIFT                  12
#define PG_4K_SIZE                   (1UL<<PG_4K_SHIFT)
#define pg_4K_offset(addr)           ((addr)&(PG_4K_SIZE-1))
#define pg_4K_nr(addr)               ((addr)>>PG_4K_SHIFT)
#define pg_4K_addr(bits)             ((bits)<<PG_4K_SHIFT)
#define pg_4K_align(addr)            __align(addr,PG_4K_SIZE)
#define pg_4K_align_next(addr)       __align_next(addr,PG_4K_SIZE)
#define pg_4K_aligned(addr)          __aligned(addr,PG_4K_SIZE)

#define PG_4M_SHIFT                  22
#define PG_4M_SIZE                   (1UL<<PG_4M_SHIFT)
#define pg_4M_offset(addr)           ((addr)&(PG_4M_SIZE-1))
#define pg_4M_nr(addr)               ((addr)>>PG_4M_SHIFT)
#define pg_4M_addr(bits)             ((bits)<<PG_4M_SHIFT)
#define pg_4M_align(addr)            __align(addr,PG_4M_SIZE)
#define pg_4M_align_next(addr)       __align_next(addr,PG_4M_SIZE)
#define pg_4M_aligned(addr)          __aligned(addr,PG_4M_SIZE)

#define pd32_idx(addr)               (((addr)>>PG_4M_SHIFT)&0x3ff)
#define pt32_idx(addr)               (((addr)>>PG_4K_SHIFT)&0x3ff)

/*
** convenient
*/
#define PAGE_SIZE                    PG_4K_SIZE
#define PAGE_SHIFT                   PG_4K_SHIFT

#define page_align(addr)             __align(addr,PAGE_SIZE)
#define page_align_next(addr)        __align_next(addr,PAGE_SIZE)
#define page_aligned(addr)           __aligned(addr,PAGE_SIZE)

#define page_nr(addr)                pg_4K_nr((offset_t)addr)
#define page_addr(bits)              pg_4K_addr((offset_t)bits)

#include <string.h>
#define __clear_page(_d)             __memset32(_d,  0, PAGE_SIZE)
#define __copy_page(_d,_s)           __memcpy32(_d, _s, PAGE_SIZE)

/*
** Invalidate 32 bits TLB entry
*/
#define invalidate(addr)             asm volatile ("invlpg %0"::"m"(addr):"memory")


/************************************************************************
 ************************* 64 bits long mode paging *********************
 ************************************************************************/

#define valid_pae_pdpe(_p)			\
   ({						\
      int x = 1;				\
      if((_p)->pae.p)				\
      {						\
	 if((_p)->pae.r0 || (_p)->pae.r1)	\
	    x = 0;				\
	 if((_p)->raw > info->vm.cpu.max_paddr)	\
	    x = 0;				\
      }						\
      x;					\
   })

typedef union page_map_level_4_entry
{
   struct
   {
      uint64_t  p:1;
      uint64_t  rw:1;
      uint64_t  lvl:1;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  acc:1;
      uint64_t  mbz:3;
      uint64_t  avl:3;
      uint64_t  addr:40; /* bit 12 */
      uint64_t  avl2:11;
      uint64_t  nx:1;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) pml4e_t;

typedef union page_directory_pointer_entry
{
   struct
   {
      uint64_t  p:1;
      uint64_t  rw:1;
      uint64_t  lvl:1;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  acc:1;
      uint64_t  mbz:3;
      uint64_t  avl:3;
      uint64_t  addr:40; /* bit 12 */
      uint64_t  avl2:11;
      uint64_t  nx:1;

   } __attribute__((packed));

   struct
   {
      uint64_t  p:1;
      uint64_t  r0:2;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  r1:4;
      uint64_t  ign:3;
      uint64_t  addr:40; /* bit 12 */
      uint64_t  r2:12;

   } __attribute__((packed)) pae; /* specific pae pdpte register */

   struct
   {
      uint64_t  p:1;
      uint64_t  rw:1;
      uint64_t  lvl:1;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  acc:1;
      uint64_t  d:1;
      uint64_t  ps:1;     /* bit 7 */
      uint64_t  g:1;
      uint64_t  avl:3;
      uint64_t  pat:1;
      uint64_t  mbz:17;
      uint64_t  addr:22;  /* bit 30 */
      uint64_t  avl2:11;
      uint64_t  nx:1;

   } __attribute__((packed)) page;

   raw64_t;

} __attribute__((packed)) pdpe_t;

typedef union page_directory_entry_64
{
   struct
   {
      uint64_t  p:1;
      uint64_t  rw:1;
      uint64_t  lvl:1;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  acc:1;
      uint64_t  mbz:3;
      uint64_t  avl:3;
      uint64_t  addr:40;   /* bit 12 */
      uint64_t  avl2:11;
      uint64_t  nx:1;

   } __attribute__((packed));

   struct
   {
      uint64_t  p:1;
      uint64_t  rw:1;
      uint64_t  lvl:1;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  acc:1;
      uint64_t  d:1;
      uint64_t  ps:1;      /* bit 7 */
      uint64_t  g:1;
      uint64_t  avl:3;
      uint64_t  pat:1;
      uint64_t  mbz:8;
      uint64_t  addr:31;   /* bit 21 */
      uint64_t  avl2:11;
      uint64_t  nx:1;

   } __attribute__((packed)) page;

   raw64_t;

} __attribute__((packed)) pde64_t;

typedef union page_table_entry_64
{
   struct
   {
      uint64_t  p:1;
      uint64_t  rw:1;
      uint64_t  lvl:1;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  acc:1;
      uint64_t  d:1;
      uint64_t  pat:1;
      uint64_t  g:1;
      uint64_t  avl:3;
      uint64_t  addr:40;  /* bit 12 */
      uint64_t  avl2:11;
      uint64_t  nx:1;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) pte64_t;

/*
** 64 bits paging usefull macros
*/
#define PML4E_PER_PML4               512UL
#define PDPE_PER_PDP                 512UL
#define PDE64_PER_PD                 512UL
#define PTE64_PER_PT                 512UL

#define PG_2M_SHIFT                  21
#define PG_2M_SIZE                   (1UL<<PG_2M_SHIFT)
#define pg_2M_offset(addr)           ((addr)&(PG_2M_SIZE-1))
#define pg_2M_nr(addr)               ((addr)>>PG_2M_SHIFT)
#define pg_2M_addr(bits)             ((bits)<<PG_2M_SHIFT)
#define pg_2M_align(addr)            __align(addr,PG_2M_SIZE)
#define pg_2M_align_next(addr)       __align_next(addr,PG_2M_SIZE)
#define pg_2M_aligned(addr)          __aligned(addr,PG_2M_SIZE)

#define PG_1G_SHIFT                  30
#define PG_1G_SIZE                   (1UL<<PG_1G_SHIFT)
#define pg_1G_offset(addr)           ((addr)&(PG_1G_SIZE-1))
#define pg_1G_nr(addr)               ((addr)>>PG_1G_SHIFT)
#define pg_1G_addr(bits)             ((bits)<<PG_1G_SHIFT)
#define pg_1G_align(addr)            __align(addr,PG_1G_SIZE)
#define pg_1G_align_next(addr)       __align_next(addr,PG_1G_SIZE)
#define pg_1G_aligned(addr)          __aligned(addr,PG_1G_SIZE)

#define PG_512G_SHIFT                39
#define PG_512G_SIZE                 (1UL<<PG_512G_SHIFT)
#define pg_512G_offset(addr)         ((addr)&(PG_512G_SIZE-1))
#define pg_512G_nr(addr)             ((addr)>>PG_512G_SHIFT)
#define pg_512G_addr(bits)           ((bits)<<PG_512G_SHIFT)
#define pg_512G_align(addr)          __align(addr,PG_512G_SIZE)
#define pg_512G_align_next(addr)     __align_next(addr,PG_512G_SIZE)
#define pg_512G_aligned(addr)        __aligned(addr,PG_512G_SIZE)

/*
** Special pdp for pmode + pae
*/
#define PG_32B_SHIFT                 5
#define PG_32B_SIZE                  (1UL<<PG_32B_SHIFT)
#define pg_32B_addr(bits)            ((bits)<<PG_32B_SHIFT)

#define pdp_pae_idx(addr)            (((addr)>>PG_1G_SHIFT)&0x3)

/*
** relative index for paging walk through
*/
#define pml4_idx(addr)               (((addr)>>PG_512G_SHIFT)&0x1ff)
#define pdp_idx(addr)                (((addr)>>PG_1G_SHIFT)&0x1ff)
#define pd64_idx(addr)               (((addr)>>PG_2M_SHIFT)&0x1ff)
#define pt64_idx(addr)               (((addr)>>PG_4K_SHIFT)&0x1ff)

/*
** absolute index:
**  - pml4_abs_idx == pdp table number
**  - pdp_abs_idx  == pd  table number
**  - pd_abs_idx   == pt  table number
*/
#define pg_abs_idx(addr,shift)       ((addr)>>(shift))
#define pdp_nr(addr)                 ((addr)>>PG_512G_SHIFT)
#define pd64_nr(addr)                ((addr)>>PG_1G_SHIFT)
#define pt64_nr(addr)                ((addr)>>PG_2M_SHIFT)

#define PML4_SZ                      (PML4E_PER_PML4*sizeof(pml4e_t))
#define PDP_SZ                       (PDPE_PER_PDP*sizeof(pdpe_t))
#define PD64_SZ                      (PDE64_PER_PD*sizeof(pde64_t))
#define PT64_SZ                      (PTE64_PER_PT*sizeof(pte64_t))

/*
** Table access
*/
typedef pml4e_t (pml4_t)[PML4E_PER_PML4];
typedef pdpe_t  (pdp_t)[PDPE_PER_PDP];
typedef pde64_t (pd64_t)[PDE64_PER_PD];
typedef pte64_t (pt64_t)[PTE64_PER_PT];

/*
** Universal macros
*/
#define pg_present(_e_)              ((_e_)->p)
#define pg_readable(_e_)             pg_present(_e_)
#define pg_writable(_e_)             (pg_present(_e_) && ((_e_)->rw))
#define pg_large(_e_)                ((_e_)->page.ps)
#define pg_zero(_e_)                 ((_e_)->raw = 0)

/* XXX: what about NX bit ? */
#define pg_get_attr(_e_)             ((_e_)->raw & (PG_USR|PG_RW))
#define pg_set_pvl(_e_,_p_)          ({(_e_)->blow &= ~7;(_e_)->blow |= (_p_);})
#define pg_has_pvl_w(_e)             ((_e_)->blow & PG_RW)



#define pg_set_entry(_e_,_attr_,_pfn_)		\
   ({						\
      (_e_)->raw  = (_attr_)|PG_P;		\
      (_e_)->addr = _pfn_;			\
   })

#define pg_set_large_entry(_e_,_attr_,_pfn_)	\
   ({						\
      (_e_)->raw       = (_attr_)|PG_PS|PG_P;	\
      (_e_)->page.addr = _pfn_;			\
   })

#endif
