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
#ifndef __VMX_EPT_H__
#define __VMX_EPT_H__

#include <types.h>
#include <pagemem.h>

/*
** VPID invalidation types
*/
#define VMCS_VPID_INV_ADDR         0
#define VMCS_VPID_INV_SINGLE_ALL   1
#define VMCS_VPID_INV_ALL          2
#define VMCS_VPID_INV_SINGLE       3

/*
** INVVPID descriptor and insn
*/
typedef struct invvpid_descriptor
{
   union
   {
      struct
      {
	 uint64_t   vpid:16;
	 uint64_t   reserved:48;

      } __attribute__((packed));

      uint64_t low;

   } __attribute__((packed));

   uint64_t   addr;         /* linear address */

} __attribute__((packed)) invvpid_desc_t;

#define invvpid(_type)							\
   ({									\
      invvpid_desc_t desc;						\
      desc.addr = desc.low = 0ULL;					\
      vmcs_read(vm_exec_ctrls.vpid);					\
      desc.vpid = (vm_exec_ctrls.vpid.raw);				\
      asm volatile("invvpid %1, %%rax"::"a"(_type),"m"(desc):"memory");	\
   })

/*
** EPT VMCS pointer
*/
typedef union vmcs_extended_page_table_pointer
{
   struct
   {
      uint64_t   cache:3;     /* 0=UC, 6=WB, others invalid */
      uint64_t   pwl:3;       /* page walk length - 1 */
      uint64_t   r1:6;        /* reserved */
      uint64_t   addr:52;     /* EPT pml4 physical addr */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcs_eptp_t;

/*
** INVEPT descriptor and insn
*/
typedef struct invept_descriptor
{
   vmcs_eptp_t eptp;
   uint64_t    r1;

} __attribute__((packed)) invept_desc_t;

/*
** EPT invalidation types
*/
#define VMCS_EPT_INV_SINGLE_ALL    1
#define VMCS_EPT_INV_ALL           2

#define invept(_type)							\
   ({									\
      invept_desc_t desc;						\
      vmcs_read(vm_exec_ctrls.eptp);					\
      desc.eptp.raw = (vm_exec_ctrls.eptp.raw);				\
      desc.r1 = 0ULL;							\
      asm volatile("invept %1, %%rax"::"a"(_type),"m"(desc):"memory");	\
   })

/*
** EPT paging structures
*/
typedef union ept_page_map_level_4_entry
{
   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  r1:5;
      uint64_t  ign:4;
      uint64_t  addr:52;   /* bit 12 */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) ept_pml4e_t;

typedef union ept_page_directory_pointer_entry
{
   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  r1:5;
      uint64_t  ign:4;
      uint64_t  addr:52;    /* bit 12 */

   } __attribute__((packed));

   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  type:3;
      uint64_t  ign_pat:1;
      uint64_t  ps:1;       /* bit 7 */
      uint64_t  ign:4;
      uint64_t  r1:18;
      uint64_t  addr:34;    /* bit 30 */

   } __attribute__((packed)) page;

   raw64_t;

} __attribute__((packed)) ept_pdpe_t;

typedef union ept_page_directory_entry_64
{
   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  r1:5;
      uint64_t  ign:4;
      uint64_t  addr:52;    /* bit 12 */

   } __attribute__((packed));

   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  type:3;
      uint64_t  ign_pat:1;
      uint64_t  ps:1;       /* bit 7 */
      uint64_t  ign:4;
      uint64_t  r1:9;
      uint64_t  addr:43;    /* bit 21 */

   } __attribute__((packed)) page;

   raw64_t;

} __attribute__((packed)) ept_pde64_t;

typedef union ept_page_table_entry_64
{
   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  type:3;
      uint64_t  ign_pat:1;
      uint64_t  ign:5;
      uint64_t  addr:52;    /* bit 12 */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) ept_pte64_t;

/*
** Macros
*/

/* use ignored bit to tell this mapping is covered by MTRR */
#define VMX_EPT_SET_MTRR_BIT         8
#define ept_has_mtrr                 ((1UL)<<VMX_EPT_SET_MTRR_BIT)

#define VMX_EPT_KEEP_MTRR_TYPE       (ept_has_mtrr|(7UL<<3))

#define VMX_EPT_MEM_TYPE_UC          0
#define VMX_EPT_MEM_TYPE_WC          1
#define VMX_EPT_MEM_TYPE_WT          4
#define VMX_EPT_MEM_TYPE_WP          5
#define VMX_EPT_MEM_TYPE_WB          6

#define VMX_EPT_PVL_RW               3
#define VMX_EPT_PVL_RX               5
#define VMX_EPT_PVL_RWX              7
#define ept_dft_pvl                  VMX_EPT_PVL_RWX

#define __ept_dft_attr(_pvl)			\
   ({						\
      uint64_t type, attr;			\
						\
      if(info->vm.mtrr_def.e)			\
	 type = info->vm.mtrr_def.type;		\
      else					\
	 type = VMX_EPT_MEM_TYPE_UC;		\
						\
      attr = (type<<3) | _pvl;			\
      attr;					\
   })

#define ept_dft_attr     __ept_dft_attr(ept_dft_pvl)
#define ept_dft_attr_nx  __ept_dft_attr(VMX_EPT_PVL_RW)

uint64_t __ept_mtrr_resolve(uint64_t, uint64_t);

#define ept_pg_get_attr(_e_)        ((_e_)->blow & 0x7f)
#define ept_pg_present(_e_)         ((ept_pg_get_attr(_e_)&0x7) != 0)

/* keep mem type and mtrr info */
#define ept_zero(_e_)				\
   ({						\
      if((_e_)->raw & ept_has_mtrr)		\
	 (_e_)->raw &= VMX_EPT_KEEP_MTRR_TYPE;	\
      else					\
	 (_e_)->raw = 0;			\
   })

#define ept_pg_set_attr(_e_,_atTr_)					\
   ({									\
      if(((_atTr_) & ept_has_mtrr) && ((_e_)->raw & ept_has_mtrr))	\
      	 (_e_)->raw = __ept_mtrr_resolve((_e_)->raw,_atTr_);		\
      else								\
	 (_e_)->raw = _atTr_;						\
   })

#define ept_pg_set_entry(_e_,_attr_,_pfn_)	\
   ({						\
      (_e_)->raw  = (_attr_) & ~(0xfUL<<3);	\
      (_e_)->addr = _pfn_;			\
   })

#define ept_pg_set_page_entry(_e_,_attr_,_pfn_)		\
   ({							\
      ept_pg_set_attr(_e_,_attr_);			\
      (_e_)->addr = _pfn_;				\
   })

#define ept_pg_set_large_page_entry(_e_,_attr_,_pfn_)	\
   ({							\
      ept_pg_set_attr(_e_,((_attr_)|PG_PS));		\
      (_e_)->page.addr = _pfn_;				\
   })


typedef ept_pml4e_t (ept_pml4_t)[PML4E_PER_PML4];
typedef ept_pdpe_t  (ept_pdp_t)[PDPE_PER_PDP];
typedef ept_pde64_t (ept_pd64_t)[PDE64_PER_PD];
typedef ept_pte64_t (ept_pt64_t)[PTE64_PER_PT];

/*
** Functions
*/
void vmx_ept_map();
void vmx_ept_unmap();
void vmx_ept_remap();

#ifndef __INIT__
int  vmx_ept_update_pdpe();
#endif

#endif
