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
#ifndef __SEGMEM_H__
#define __SEGMEM_H__

#include <types.h>
#include <gpr.h>

typedef struct descriptor_table_register
{
   uint16_t  limit;
   raw64_t   base;

} __attribute__((packed)) dt_reg_t;

/*
** Segment Selector
*/
#define SEG_SEL_GDT    0
#define SEG_SEL_LDT    1

#define SEG_SEL_KRN    0
#define SEG_SEL_USR    3

typedef union segment_selector
{
   struct
   {
      uint16_t    rpl:2;        /* bits 0-1  requested privilege level */
      uint16_t    ti:1;         /* bit  2    table indicator gdt(0) or ldt(1) */
      uint16_t    index:13;     /* bits 3-15 index in descriptor table */

   } __attribute__((packed));

   uint16_t raw;

} __attribute__((packed)) seg_sel_t;

#define dt_seg_sel(idx,ti,rpl)      (((idx)<<3)|((ti)<<2)|(rpl))
#define gdt_seg_sel(idx,rpl)        dt_seg_sel(idx,SEG_SEL_GDT,rpl)
#define ldt_seg_sel(idx,rpl)        dt_seg_sel(idx,SEG_SEL_LDT,rpl)
#define gdt_krn_seg_sel(idx)        gdt_seg_sel(idx,SEG_SEL_KRN)
#define gdt_usr_seg_sel(idx)        gdt_seg_sel(idx,SEG_SEL_USR)

/*
** Segment Descriptor
*/
#define SEG_DESC_DATA_R             0x0
#define SEG_DESC_DATA_RA            0x1
#define SEG_DESC_DATA_RW            0x2
#define SEG_DESC_DATA_RWA           0x3
#define SEG_DESC_DATA_ER            0x4
#define SEG_DESC_DATA_ERA           0x5
#define SEG_DESC_DATA_ERW           0x6
#define SEG_DESC_DATA_ERWA          0x7

#define SEG_DESC_CODE_X             0x8
#define SEG_DESC_CODE_XA            0x9
#define SEG_DESC_CODE_XR            0xa
#define SEG_DESC_CODE_XRA           0xb
#define SEG_DESC_CODE_CX            0xc
#define SEG_DESC_CODE_CXA           0xd
#define SEG_DESC_CODE_CXR           0xe
#define SEG_DESC_CODE_CXRA          0xf

#define SEG_DESC_SYS_TSS_AVL_16     0x1
#define SEG_DESC_SYS_LDT            0x2
#define SEG_DESC_SYS_TSS_BUSY_16    0x3
#define SEG_DESC_SYS_CALL_GATE_16   0x4
#define SEG_DESC_SYS_TASK_GATE      0x5
#define SEG_DESC_SYS_INTR_GATE_16   0x6
#define SEG_DESC_SYS_TRAP_GATE_16   0x7
#define SEG_DESC_SYS_TSS_AVL_32     0x9
#define SEG_DESC_SYS_TSS_BUSY_32    0xb
#define SEG_DESC_SYS_CALL_GATE_32   0xc
#define SEG_DESC_SYS_INTR_GATE_32   0xe
#define SEG_DESC_SYS_TRAP_GATE_32   0xf

#define SEG_DESC_SYS_LDT_64         0x2
#define SEG_DESC_SYS_TSS_AVL_64     0x9
#define SEG_DESC_SYS_TSS_BUSY_64    0xb
#define SEG_DESC_SYS_CALL_GATE_64   0xc
#define SEG_DESC_SYS_INTR_GATE_64   0xe
#define SEG_DESC_SYS_TRAP_GATE_64   0xf

typedef union segment_descriptor
{
   raw64_t;

   struct
   {
      uint64_t    limit_1:16;          /* bits 00-15 of the segment limit */
      uint64_t    base_1:16;           /* bits 00-15 of the base address */
      uint64_t    base_2:8;            /* bits 16-23 of the base address */
      uint64_t    type:4;              /* segment type */
      uint64_t    s:1;                 /* descriptor type */
      uint64_t    dpl:2;               /* descriptor privilege level */
      uint64_t    p:1;                 /* segment present flag */
      uint64_t    limit_2:4;           /* bits 16-19 of the segment limit */
      uint64_t    avl:1;               /* available for fun and profit */
      uint64_t    l:1;                 /* longmode */
      uint64_t    d:1;                 /* default length, depend on seg type */
      uint64_t    g:1;                 /* granularity */
      uint64_t    base_3:8;            /* bits 24-31 of the base address */

   } __attribute__((packed));

} __attribute__((packed)) seg_desc_t;

typedef struct system_segment_descriptor_64
{
   seg_desc_t;
   uint32_t base_4;                    /* bits 32-63 of the base address */
   uint32_t zero;

} __attribute__((packed)) sys64_seg_desc_t;

#define null_desc                 (0x0000000000000000ULL)
#define code32_desc               (0x00cf9b000000ffffULL)
#define code64_desc               (0x00af9b000000ffffULL)
#define data32_desc               (0x00cf93000000ffffULL)

/*
** Global descriptor table
*/
typedef struct global_descriptor_table_register
{
   uint16_t            limit;           /* dt limit = size - 1 */
   union                                /* base address */
   {
      offset_t         addr;
      seg_desc_t       *desc;
      sys64_seg_desc_t *ldesc;
   };

} __attribute__((packed)) gdt_reg_t;

/*
** Local descriptor table
*/
typedef struct local_descriptor_table_register
{
   seg_sel_t           sel;      /* segment selector */
   uint16_t            limit;    /* segment limit */
   union                         /* base address */
   {
      offset_t         addr;
      seg_desc_t       *desc;
      sys64_seg_desc_t *ldesc;
   };

} __attribute__((packed)) ldt_reg_t;

/*
** Interrupt descriptor
*/
typedef union interrupt_descriptor
{
   raw64_t;

   struct
   {
      uint64_t  offset_1:16;    /* bits 00-15 of the isr offset */
      uint64_t  selector:16;    /* isr segment selector */
      uint64_t  ist:3;          /* stack table: only 64 bits */
      uint64_t  zero_1:5;       /* must be 0 */
      uint64_t  type:4;         /* interrupt/trap gate */
      uint64_t  zero_2:1;       /* must be zero */
      uint64_t  dpl:2;          /* privilege level */
      uint64_t  p:1;            /* present flag */
      uint64_t  offset_2:16;    /* bits 16-31 of the isr offset */

   } __attribute__((packed));

} __attribute__((packed)) int_desc_t;

typedef struct interrupt_descriptor_64
{
   int_desc_t;
   uint32_t offset_3;           /* bits 32-63 of the isr offset */
   uint32_t zero_3;

} __attribute__((packed)) int64_desc_t;

#define int32_desc(_dsc_, _cs_, _isr_)					\
   ({									\
      raw32_t addr32;							\
      addr32.raw        = _isr_;					\
      (_dsc_)->raw      = addr32.wlow;					\
      (_dsc_)->selector = _cs_;						\
      (_dsc_)->type     = SEG_DESC_SYS_INTR_GATE_32;			\
      (_dsc_)->offset_2 = addr32.whigh;					\
      (_dsc_)->p        = 1;						\
   })

#define int64_desc(_dsc_, _cs_, _isr64_)				\
   ({									\
      raw64_t addr64;							\
      addr64.raw = _isr64_;						\
      (_dsc_)->offset_3 = addr64.high;					\
      int32_desc(_dsc_, _cs_, addr64.low);				\
   })

/*
** Real-mode interrupt vector table entry
*/
typedef union interrupt_vector_table_entry
{
   struct
   {
      uint16_t ip;
      uint16_t cs;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ivt_e_t;

/*
** Interrupt descriptor table
*/
typedef struct interrupt_descriptor_table_register
{
   uint16_t        limit;           /* dt limit = size - 1 */
   union                            /* base address */
   {
      offset_t     addr;
      int_desc_t   *desc;
      int64_desc_t *ldesc;
   };

} __attribute__((packed)) idt_reg_t;

/*
** Task state segment structures
*/
typedef struct task_state_segment_stack
{
   uint32_t esp;
   uint16_t ss;
   uint16_t pad;

} __attribute__((packed)) tss_stack_t;

typedef struct task_state_segment_maps
{
   uint16_t addr;
   uint8_t  intr[32];
   uint8_t  io[8192];
   uint8_t  boundary;

} __attribute__((packed)) tss_maps_t;

typedef struct task_state_segment_map_64
{
   uint16_t addr;
   uint8_t  io[8192];
   uint8_t  boundary;

} __attribute__((packed)) tss64_map_t;

typedef union task_state_segment_trap
{
   struct
   {
      uint16_t on:1;   /* #DB on task switch */
      uint16_t pad:15;

   } __attribute__((packed));

   uint16_t    raw;

} __attribute__((packed)) tss_trap_t;

/*
** 32 bits TSS
*/
typedef struct task_state_segment
{
   uint16_t     back_link, r1;

   tss_stack_t  s0;
   tss_stack_t  s1;
   tss_stack_t  s2;

   uint32_t     cr3;
   uint32_t     eip;
   eflags_reg_t eflags;

   gpr32_t      gpr;

   uint16_t     es, r5;
   uint16_t     cs, r6;
   uint16_t     ss, r7;
   uint16_t     ds, r8;
   uint16_t     fs, r9;
   uint16_t     gs, r10;
   uint16_t     ldt, r11;

   tss_trap_t   trap;
   tss_maps_t   maps;

} __attribute__((packed)) tss_t;

/*
** 64 bits TSS
*/
typedef struct task_state_segment_64
{
   uint32_t    rsv0;

   uint64_t    rsp0;
   uint64_t    rsp1;
   uint64_t    rsp2;

   uint64_t    rsv1;

   uint64_t    ist1;
   uint64_t    ist2;
   uint64_t    ist3;
   uint64_t    ist4;
   uint64_t    ist5;
   uint64_t    ist6;
   uint64_t    ist7;

   uint64_t    rsv2;
   uint16_t    rsv3;

   tss64_map_t map;

} __attribute__((packed)) tss64_t;

#define tss_idx(_idx_)                  (1<<((_idx_)%8))
#define tss_allow(tSs_,mAp,iDx)         ((tSs_)->mAp[(iDx)/8] &= ~tss_idx(iDx))
#define tss_deny(tSs,mAp,iDx)           ((tSs)->mAp[(iDx)/8] |=  tss_idx(iDx))
#define tss_is_denied(tSs,mAp,iDx)      (((tSs)->mAp[(iDx)/8]&tss_idx(iDx))?1:0)

#define tss_allow_int(_tss_,_idx_)      tss_allow(_tss_,int_bitmap,_idx_)
#define tss_deny_int(_tss_,_idx_)       tss_deny(_tss_,int_bitmap,_idx_)
#define tss_is_denied_int(_tss_,_idx_)  tss_is_denied(_tss_,int_bitmap,_idx_)

#define tss_allow_io(_tss_,_idx_)       tss_allow(_tss_,io_bitmap,_idx_)
#define tss_deny_io(_tss_,_idx_)        tss_deny(_tss_,io_bitmap,_idx_)
#define tss_is_denied_io(_tss_,_idx_)   tss_is_denied_io(_tss_,io_bitmap,_idx_)

#define __tss_desc(dsc,_aDdr_,_sz_)				\
   ({								\
      dsc->raw    = _sz_;					\
      dsc->base_1 = _aDdr_.wlow;				\
      dsc->base_2 = _aDdr_._whigh.blow;				\
      dsc->base_3 = _aDdr_._whigh.bhigh;			\
      dsc->type   = SEG_DESC_SYS_TSS_AVL_32;			\
      dsc->p      = 1;						\
   })

#define tss32_desc(dsc,_tss_)   __tss_desc(dsc, _tss_, sizeof(tss_t))

#define tss64_desc(dsc64,_tss_)					\
   ({								\
      raw64_t addr64;						\
      addr64.raw = _tss_;					\
      __tss_desc(dsc64, addr64._low, sizeof(tss64_t));		\
      dsc64->base_4 = addr64.high;				\
      dsc64->zero = 0ULL;					\
   })

/*
** Various segmentation related cpu instructions
*/
#define get_seg_sel(_reg_)						\
   ({									\
      uint16_t seg;							\
      asm volatile (							\
	 "xor %%eax, %%eax        \n"					\
	 "mov %%"##_reg_##", %%ax \n"					\
	 :"=a"(seg));							\
      seg;								\
   })

#define get_ss()                  get_seg_sel(ss)
#define get_fs()                  get_seg_sel(fs)
#define get_gs()                  get_seg_sel(gs)

#define get_gdtr(aLocation)       asm volatile ("sgdtl %0"::"m"(aLocation):"memory")
#define get_ldtr(aLocation)       asm volatile ("sldtl %0"::"m"(aLocation):"memory")
#define get_idtr(aLocation)       asm volatile ("sidtl %0"::"m"(aLocation):"memory")
#define get_tr(aLocation)         asm volatile ("str   %0"::"m"(aLocation):"memory")

#define set_gdtr(val)             asm volatile ("lgdt  %0"::"m"(val):"memory")
#define set_ldtr(val)             asm volatile ("lldt  %0"::"m"(val):"memory")
#define set_idtr(val)             asm volatile ("lidt  %0"::"m"(val):"memory")
#define set_tr(val)               asm volatile ("ltr   %%ax"::"a"(val))

#define farjump(_fptr)            asm volatile ("ljmp  *%0"::"m"(_fptr):"memory");
#define set_cs_eip(_cs,_eip)      asm volatile ("ljmp  %0, %1"::"i"(_cs), "i"(_eip))

#ifndef __X86_64__
#define set_cs(_cs)               asm volatile ("ljmp  %0, $1f ; 1:"::"i"(_cs))
#else
#define set_cs(_cs)					\
   ({							\
      fptr32_t addr;					\
      addr.segment = _cs;				\
      asm volatile (					\
	 "movl  $1f, %0 \n"				\
	 "ljmp  *%0     \n"				\
	 "1:            \n"				\
	 ::"m"(addr.offset):"memory");			\
   })
#endif

#define segmem_reload(_cs,_ds)			\
   ({						\
      set_cs(_cs);				\
      asm volatile (				\
	 "movw   %%ax, %%ss  \n"		\
	 "movw   %%ax, %%ds  \n"		\
	 "movw   %%ax, %%es  \n"		\
	 "movw   %%ax, %%fs  \n"		\
	 "movw   %%ax, %%gs  \n"		\
	 ::"a"(_ds));				\
   })

#endif
