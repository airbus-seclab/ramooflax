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
#ifndef __CR_H__
#define __CR_H__

#include <types.h>

/*
** CR0
*/
#define CR0_PE_BIT      0
#define CR0_MP_BIT      1
#define CR0_EM_BIT      2
#define CR0_TS_BIT      3
#define CR0_ET_BIT      4
#define CR0_NE_BIT      5
#define CR0_WP_BIT     16
#define CR0_AM_BIT     18
#define CR0_NW_BIT     29
#define CR0_CD_BIT     30
#define CR0_PG_BIT     31

#define CR0_PE         (1UL<<CR0_PE_BIT)
#define CR0_MP         (1UL<<CR0_MP_BIT)
#define CR0_EM         (1UL<<CR0_EM_BIT)
#define CR0_TS         (1UL<<CR0_TS_BIT)
#define CR0_ET         (1UL<<CR0_ET_BIT)
#define CR0_NE         (1UL<<CR0_NE_BIT)
#define CR0_WP         (1UL<<CR0_WP_BIT)
#define CR0_AM         (1UL<<CR0_AM_BIT)
#define CR0_NW         (1UL<<CR0_NW_BIT)
#define CR0_CD         (1UL<<CR0_CD_BIT)
#define CR0_PG         (1UL<<CR0_PG_BIT)

typedef union control_register_0
{
   struct
   {
      uint64_t    pe:1;   /* protected mode */
      uint64_t    mp:1;   /* monitor copro */
      uint64_t    em:1;   /* emulation */
      uint64_t    ts:1;   /* task switch */
      uint64_t    et:1;   /* ext type */
      uint64_t    ne:1;   /* num error */
      uint64_t    r1:10;  /* reserved */
      uint64_t    wp:1;   /* write protect */
      uint64_t    r2:1;   /* reserved */
      uint64_t    am:1;   /* align mask */
      uint64_t    r3:10;  /* reserved */
      uint64_t    nw:1;   /* not write through */
      uint64_t    cd:1;   /* cache disable */
      uint64_t    pg:1;   /* paging */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) cr0_reg_t;

/*
** XCR0
*/
typedef union extended_control_register_0
{
   struct
   {
      uint64_t    x87:1;   /* FPU/MMX state must be 1 */
      uint64_t    sse:1;   /* SSE state */
      uint64_t    avx:1;   /* AVX state */
      uint64_t    rsv:61;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) xcr0_reg_t;

/*
** CR3
*/
typedef union control_register_3
{
   struct
   {
      uint64_t  r1:3;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  r2:7;
      uint64_t  addr:20;

   } __attribute__((packed));

   struct
   {
      uint64_t  r1:3;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  addr:27;

   } __attribute__((packed)) pae;

   struct
   {
      uint64_t  r1:3;
      uint64_t  pwt:1;
      uint64_t  pcd:1;
      uint64_t  r2:7;
      uint64_t  addr:40; /* bit 12 */

   } __attribute__((packed)) pml4;

   raw64_t;

} __attribute__((packed)) cr3_reg_t;

/*
** CR4
*/
#define CR4_VME_BIT          0
#define CR4_PVI_BIT          1
#define CR4_TSD_BIT          2
#define CR4_DE_BIT           3
#define CR4_PSE_BIT          4
#define CR4_PAE_BIT          5
#define CR4_MCE_BIT          6
#define CR4_PGE_BIT          7
#define CR4_PCE_BIT          8
#define CR4_OSFXSR_BIT       9
#define CR4_OSXMMEXCPT_BIT  10
#define CR4_VMXE_BIT        13
#define CR4_SMXE_BIT        14
#define CR4_PCIDE_BIT       17
#define CR4_OSXSAVE_BIT     18
#define CR4_SMEP_BIT        20

#define CR4_VME             (1UL<<CR4_VME_BIT)
#define CR4_PVI             (1UL<<CR4_PVI_BIT)
#define CR4_TSD             (1UL<<CR4_TSD_BIT)
#define CR4_DE              (1UL<<CR4_DE_BIT)
#define CR4_PSE             (1UL<<CR4_PSE_BIT)
#define CR4_PAE             (1UL<<CR4_PAE_BIT)
#define CR4_MCE             (1UL<<CR4_MCE_BIT)
#define CR4_PGE             (1UL<<CR4_PGE_BIT)
#define CR4_PCE             (1UL<<CR4_PCE_BIT)
#define CR4_OSFXSR          (1UL<<CR4_OSFXSR_BIT)
#define CR4_OSXMMEXCPT      (1UL<<CR4_OSXMMEXCPT_BIT)
#define CR4_VMXE            (1UL<<CR4_VMXE_BIT)
#define CR4_SMXE            (1UL<<CR4_SMXE_BIT)
#define CR4_PCIDE           (1UL<<CR4_PCIDE_BIT)
#define CR4_OSXSAVE         (1UL<<CR4_OSXSAVE_BIT)
#define CR4_SMEP            (1UL<<CR4_SMEP_BIT)

typedef union control_register_4
{
   struct
   {
      uint64_t    vme:1;        /* virtual 8086 */
      uint64_t    pvi:1;        /* pmode virtual int */
      uint64_t    tsd:1;        /* time stamp disable */
      uint64_t    de:1;         /* debug ext */
      uint64_t    pse:1;        /* page sz ext */
      uint64_t    pae:1;        /* phys addr ext */
      uint64_t    mce:1;        /* machine check enable */
      uint64_t    pge:1;        /* page global enable */
      uint64_t    pce:1;        /* perf counter enable */
      uint64_t    osfxsr:1;     /* fxsave fxstore */
      uint64_t    osxmmexcpt:1; /* simd fpu excpt */
      uint64_t    r1:2;         /* reserved */
      uint64_t    vmxe:1;       /* vmx enable */
      uint64_t    smxe:1;       /* smx enable */
      uint64_t    r2:2;         /* reserved */
      uint64_t    pcide:1;      /* process context id */
      uint64_t    osxsave:1;    /* xsave enable */
      uint64_t    r3:1;         /* reserved */
      uint64_t    smep:1;       /* smep */
      uint64_t    r4:11;        /* smep reserved */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) cr4_reg_t;


/*
** Mov to/from cr are exclusive:
**  - if 64 bits -> get 64 bits
**  - if 32 bits -> get 32 bits
*/
#define get_cr(_n_)     ({ulong_t x;asm volatile("mov %%cr" #_n_ ", %0":"=r"(x));x;})
#define get_cr0()       get_cr(0)
#define get_cr2()       get_cr(2)
#define get_cr3()       get_cr(3)
#define get_cr4()       get_cr(4)

#define set_cr(_n_,_x_) asm volatile("mov %0, %%cr" #_n_ ::"r"(_x_))
#define set_cr0(x)      set_cr(0,x)
#define set_cr2(x)      set_cr(2,x)
#define set_cr3(x)      set_cr(3,x)
#define set_cr4(x)      set_cr(4,x)

#define enable_paging() ({ulong_t cr0 = get_cr0(); set_cr0(cr0|CR0_PG);})
#define enable_vme()    ({ulong_t cr4 = get_cr4(); set_cr4(cr4|CR4_VME);})
#define disable_vme()   ({ulong_t cr4 = get_cr4(); set_cr4(cr4&(~CR4_VME));})

/*
** Functions
*/
#ifndef __INIT__

#define __valid_cr_regs(_gpr,_cr)     (_gpr <= GPR64_RAX && _cr <= 4 && _cr != 1)
#define __valid_cr_access()           (__rmode() || !__cpl)

#define __invalid_cr0_setup(_cr)				\
   (((_cr)->pg && !(_cr)->pe) || ((_cr)->nw && !(_cr)->cd))

#define __invalid_cr0_lmode_check(_cr,_update)				\
   (((_update)&CR0_PG) && (_cr)->pg &&					\
    __efer.lme && (!__cr4.pae || __cs.attributes.l))

int  __resolve_cr0_wr(cr0_reg_t*);
int  __resolve_cr3_wr(cr3_reg_t*);
int  __resolve_cr4_wr(cr4_reg_t*);

int __resolve_cr_wr_with(uint8_t, raw64_t*);
int  __resolve_cr(uint8_t, uint8_t, uint8_t);
#endif

#endif
