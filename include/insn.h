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
#ifndef __INSN_H__
#define __INSN_H__

#include <types.h>

#include <config.h>
#ifdef CONFIG_ARCH_AMD
#include <amd.h>
#else
#include <intel.h>
#endif

/*
** Some x86 insn definitions
*/
#define BREAKPOINT_INSN       0xcc
#define SYSENTER_INSN         0x340f

#define MOV_CR_INSN_SZ        3
#define HLT_INSN_SZ           1
#define CPUID_INSN_SZ         2
#define MSR_INSN_SZ           2
#define INVD_INSN_SZ          2
#define WBINVD_INSN_SZ        2

#define X86_MAX_INSN_LEN     15

/* group1 prefix */
#define X86_PREFIX_REP       0xf3
/* group2 prefix */
#define X86_PREFIX_CS        0x2e
#define X86_PREFIX_SS        0x36
#define X86_PREFIX_DS        0x3e
#define X86_PREFIX_ES        0x26
#define X86_PREFIX_FS        0x64
#define X86_PREFIX_GS        0x65

/* group3/4 prefix force 32 or 16 bits depending on execution mode */
#define X86_PREFIX_OP        0x66
#define X86_PREFIX_ADDR      0x67

/* some string opcodes */
#define X86_OPCODE_INS8      0x6c
#define X86_OPCODE_INS32     0x6d

#define X86_OPCODE_OUTS8     0x6e
#define X86_OPCODE_OUTS32    0x6f

#define X86_OPCODE_STOS8     0xaa
#define X86_OPCODE_STOS32    0xab

#define X86_OPCODE_LODS8     0xac
#define X86_OPCODE_LODS32    0xad

#define X86_OPCODE_MOVS8     0xa4
#define X86_OPCODE_MOVS32    0xa5

/*
** rdtsc
*/
#define rdtsc()							\
   ({								\
      raw64_t x;						\
      asm volatile ("lfence;rdtsc":"=a"(x.low),"=d"(x.high));	\
      x.raw;							\
   })

/*
** read/write to io ports
*/
#define insb(_d_,_p_)   asm volatile ("insb"::"D"(_d_),"d"(_p_))
#define outsb(_d_,_p_)  asm volatile ("outsb"::"S"(_d_),"d"(_p_))

#define outb(_d_,_p_)   asm volatile ("outb %%al,  %%dx"::"a"(_d_),"d"(_p_))
#define outw(_d_,_p_)   asm volatile ("outw %%ax,  %%dx"::"a"(_d_),"d"(_p_))
#define outl(_d_,_p_)   asm volatile ("outl %%eax, %%dx"::"a"(_d_),"d"(_p_))

#define inb(_p_)						\
   ({								\
      uint8_t _d_;						\
      asm volatile ("inb %%dx,%%al":"=a"(_d_):"d"(_p_));	\
      _d_;							\
   })

#define inw(_p_)						\
   ({								\
      uint16_t _d_;						\
      asm volatile ("inw %%dx,%%ax":"=a"(_d_):"d"(_p_));	\
      _d_;							\
   })

#define inl(_p_)						\
   ({								\
      uint32_t _d_;						\
      asm volatile("inl %%dx, %%eax":"=a"(_d_):"d"(_p_));	\
      _d_;							\
   })

#define out(_d,_p)  outb(_d,_p)
#define in(_p)      inb(_p)

/*
** Barbarian halt
*/
#define __halt()  ({ asm volatile ("hlt"); })

/*
** Lock VMM
*/
#define lock_vmm() ({ while(1) { __halt(); force_interrupts_off(); } })

/*
** 16/32/64 bits swap
*/
#define swap16(_x_)						\
   ({								\
      uint16_t _v_;						\
      asm volatile ("ror $8, %%ax":"=a"(_v_):"a"(_x_));		\
      _v_;							\
   })

#define swap32(_x_)					\
   ({							\
      uint32_t _v_;					\
      asm volatile ("bswap %%eax":"=a"(_v_):"a"(_x_));	\
      _v_;						\
   })

#define swap64(_x_)					\
   ({							\
      uint64_t _v_;					\
      asm volatile ("bswap %%rax":"=a"(_v_):"a"(_x_));	\
      _v_;						\
   })

/*
** Asm to C insn
*/
#define invd()      ({ asm volatile ("invd");   })
#define wbinvd()    ({ asm volatile ("wbinvd"); })

/*
** functions
*/
#ifndef __INIT__
int  resolve_hypercall();
int  resolve_invd();
int  resolve_wbinvd();
int  resolve_icebp();
int  resolve_hlt();
int  resolve_default();
#endif


#endif
