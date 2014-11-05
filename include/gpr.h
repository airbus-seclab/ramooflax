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
#ifndef __GPR_H__
#define __GPR_H__

#include <types.h>

/*
** RFLAGS
*/
#define RFLAGS_CF_BIT         0
#define RFLAGS_PF_BIT         2
#define RFLAGS_AF_BIT         4
#define RFLAGS_ZF_BIT         6
#define RFLAGS_SF_BIT         7
#define RFLAGS_TF_BIT         8
#define RFLAGS_IF_BIT         9
#define RFLAGS_DF_BIT        10
#define RFLAGS_OF_BIT        11
#define RFLAGS_VM_BIT        17
#define RFLAGS_VIF_BIT       19
#define RFLAGS_VIP_BIT       20

#define RFLAGS_CF            (1<<RFLAGS_CF_BIT)
#define RFLAGS_PF            (1<<RFLAGS_PF_BIT)
#define RFLAGS_AF            (1<<RFLAGS_AF_BIT)
#define RFLAGS_ZF            (1<<RFLAGS_ZF_BIT)
#define RFLAGS_SF            (1<<RFLAGS_SF_BIT)
#define RFLAGS_TF            (1<<RFLAGS_TF_BIT)
#define RFLAGS_IF            (1<<RFLAGS_IF_BIT)
#define RFLAGS_DF            (1<<RFLAGS_DF_BIT)
#define RFLAGS_OF            (1<<RFLAGS_OF_BIT)
#define RFLAGS_VM            (1<<RFLAGS_VM_BIT)
#define RFLAGS_VIF           (1<<RFLAGS_VIF_BIT)
#define RFLAGS_VIP           (1<<RFLAGS_VIP_BIT)

#define RFLAGS_IOPL          (3<<12)

typedef struct cpu_eflags_register_fields
{
   uint32_t   cf:1;     /* carry */
   uint32_t   r1:1;     /* (1) */
   uint32_t   pf:1;     /* parity */
   uint32_t   r2:1;     /* (0) */
   uint32_t   af:1;     /* adjust */
   uint32_t   r3:1;     /* (0) */
   uint32_t   zf:1;     /* zero */
   uint32_t   sf:1;     /* sign */

   uint32_t   tf:1;     /* trap */
   uint32_t   IF:1;     /* int */
   uint32_t   df:1;     /* div */
   uint32_t   of:1;     /* overflow */
   uint32_t   iopl:2;   /* io pvl */
   uint32_t   nt:1;     /* nested task */
   uint32_t   r4:1;     /* (0) */

   uint32_t   rf:1;     /* resume */
   uint32_t   vm:1;     /* virtual 8086 */
   uint32_t   ac:1;     /* align */
   uint32_t   vif:1;    /* virtual IF */

   uint32_t   vip:1;    /* virtual Interrupt Pending */
   uint32_t   id:1;     /* identification */
   uint32_t   r5:10;    /* (0) */

} __attribute__((packed)) eflags_reg_fields_t;

typedef union cpu_rflags_register
{
   eflags_reg_fields_t;
   raw64_t;

} __attribute__((packed)) rflags_reg_t;

typedef union cpu_eflags_register
{
   eflags_reg_fields_t;
   raw32_t;

} __attribute__((packed)) eflags_reg_t;

typedef union cpu_xflags_register
{
   eflags_reg_fields_t;
   ulong_t raw;

} __attribute__((packed)) xflags_reg_t;

/*
** Save/Restore cpu flags (32/64)
*/
#define save_flags(flags)    asm volatile( "pushf;pop %0":"=m"(flags)::"memory" )
#define load_flags(flags)    asm volatile( "push %0;popf"::"m"(flags):"memory","cc" )
#define clear_flags()        asm volatile( "push $0 ; popf":::"cc" );
#define get_flags()          ({ulong_t flg; save_flags(flg); flg;})

/*
** General Purpose Registers
*/
typedef union general_purpose_registers
{
   struct
   {
      raw32_t   eax;
      raw32_t   ecx;
      raw32_t   edx;
      raw32_t   ebx;
      raw32_t   esp;
      raw32_t   ebp;
      raw32_t   esi;
      raw32_t   edi;

   } __attribute__((packed));

   raw32_t      raw[8];

} __attribute__((packed)) gpr32_t;

typedef union general_purpose_registers_64
{
   struct
   {
      raw64_t   rax;
      raw64_t   rcx;
      raw64_t   rdx;
      raw64_t   rbx;
      raw64_t   rsp;
      raw64_t   rbp;
      raw64_t   rsi;
      raw64_t   rdi;
      raw64_t   r8;
      raw64_t   r9;
      raw64_t   r10;
      raw64_t   r11;
      raw64_t   r12;
      raw64_t   r13;
      raw64_t   r14;
      raw64_t   r15;

   } __attribute__((packed));

   raw64_t      raw[16];

} __attribute__((packed)) gpr64_t;

/*
** GPRs context after "pusha"
*/
typedef union general_purpose_registers_context
{
   struct
   {
      raw32_t   edi;
      raw32_t   esi;
      raw32_t   ebp;
      raw32_t   esp;
      raw32_t   ebx;
      raw32_t   edx;
      raw32_t   ecx;
      raw32_t   eax;

   } __attribute__((packed));

   raw32_t      raw[8];

} __attribute__((packed)) gpr32_ctx_t;

/*
** GPRs context after software "pushq"
*/
#define GPR64_RAX 15
#define GPR64_RCX 14
#define GPR64_RDX 13
#define GPR64_RBX 12
#define GPR64_RSP 11
#define GPR64_RBP 10
#define GPR64_RSI  9
#define GPR64_RDI  8
#define GPR64_R8   7
#define GPR64_R9   6
#define GPR64_R10  5
#define GPR64_R11  4
#define GPR64_R12  3
#define GPR64_R13  2
#define GPR64_R14  1
#define GPR64_R15  0

typedef union general_purpose_registers_context_64
{
   struct
   {
      raw64_t   r15;
      raw64_t   r14;
      raw64_t   r13;
      raw64_t   r12;
      raw64_t   r11;
      raw64_t   r10;
      raw64_t   r9;
      raw64_t   r8;
      raw64_t   rdi;
      raw64_t   rsi;
      raw64_t   rbp;
      raw64_t   rsp;
      raw64_t   rbx;
      raw64_t   rdx;
      raw64_t   rcx;
      raw64_t   rax;

   } __attribute__((packed));

   raw64_t      raw[16];

} __attribute__((packed)) gpr64_ctx_t;

/*
** General Purpose Register operations
*/
#define get_reg(_r_,_t_)						\
   ({									\
      _t_ v;								\
      asm volatile( "mov %%"#_r_", %0":"=m"(v)::"memory" );		\
      v;								\
   })

#define get_rip()         ({offset_t x; asm volatile ("lea 0(%%rip), %%rax":"=a"(x)); x;})

#define set_reg(_r_,_v_)  asm volatile ("mov %0, %%"#_r_::"m"(_v_):"memory")
#define get_reg32(_R_)    get_reg(_R_, uint32_t)
#define get_reg64(_R_)    get_reg(_R_, uint64_t)

#define set_edi(val)      set_reg(edi,val)
#define set_ebp(val)      set_reg(ebp,val)
#define set_esp(val)      set_reg(esp,val)
#define set_rbp(val)      set_reg(rbp,val)
#define set_rsp(val)      set_reg(rsp,val)

#define get_ebp()         get_reg32(ebp)
#define get_esp()         get_reg32(esp)
#define get_rbp()         get_reg64(rbp)
#define get_rsp()         get_reg64(rsp)

#endif
