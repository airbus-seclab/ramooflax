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
#ifndef __MSR_H__
#define __MSR_H__

#include <config.h>
#include <types.h>

/*
** Used for MSR reading and writing
**
** In 64 bits mode with REX prefix
** upper dword of rax and rdx is 0
**
** MSR is still stored into eax,edx
*/
typedef union raw_msr_entry
{
   struct
   {
      uint32_t  eax;
      uint32_t  edx;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) msr_t;

/*
** Read/Write standard MSR (ecx=index, edx,eax=value)
*/
#define rd_msr32(iNdex,vAl)    asm volatile("rdmsr":"=a"((vAl)):"c"((iNdex)):"edx")
#define wr_msr32(iNdex,vAl)    asm volatile("wrmsr"::"c"((iNdex)),"a"((vAl)))

#define rd_msr64(iNdex,hiGh,loW)				\
   asm volatile("rdmsr" :"=d"((hiGh)),"=a"((loW)):"c"((iNdex)))

#define wr_msr64(iNdex,hiGh,loW)					\
     asm volatile("wrmsr" ::"d"((hiGh)),"a"((loW)),"c"((iNdex)))

#define __rd_msr(eax,ecx,edx)            rd_msr64(ecx,edx,eax)
#define __wr_msr(eax,ecx,edx)            wr_msr64(ecx,edx,eax)

/*
** IA32_DEBUG_CTL_MSR (cf. vmx/svm msr)
*/
#define IA32_DEBUG_CTL_MSR                 0x1d9UL

/*
** IA32_APIC_BASE_MSR
*/
#define IA32_APIC_BASE_MSR                 0x1bUL
typedef union ia32_apic_base_msr
{
   struct
   {
      uint64_t    rsrvd0:8;
      uint64_t    bsp:1;
      uint64_t    rsrvd1:2;
      uint64_t    enable:1;
      uint64_t    paddr:24;
      uint64_t    rsrvd2:28;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) ia32_apic_base_msr_t;

#define rd_msr_ia32_apic_base(val)			\
   rd_msr64(IA32_APIC_BASE_MSR,(val).edx,(val).eax)
#define wr_msr_ia32_apic_base(val)			\
   wr_msr64(IA32_APIC_BASE_MSR,(val).edx,(val).eax)

#define apic_set_enable(_x_)					\
   ({								\
      ia32_apic_base_msr_t apic;				\
      rd_msr_ia32_apic_base(apic);				\
      apic.enable = (_x_);					\
      wr_msr_ia32_apic_base(apic);				\
   })

#define apic_enable()    apic_set_enable(1)
#define apic_disable()   apic_set_enable(0)

/*
** IA32_SYSENTER_CS, IA32_SYSENTER_EIP, IA32_SYSENTER_ESP
*/
#define IA32_SYSENTER_CS_MSR                0x174UL
#define IA32_SYSENTER_ESP_MSR               0x175UL
#define IA32_SYSENTER_EIP_MSR               0x176UL

#define rd_msr_ia32_sysenter_cs(val)      rd_msr32(IA32_SYSENTER_CS_MSR,val.raw)
#define rd_msr_ia32_sysenter_esp(val)			\
   rd_msr64(IA32_SYSENTER_ESP_MSR,(val).high,(val).low)
#define rd_msr_ia32_sysenter_eip(val)			\
   rd_msr64(IA32_SYSENTER_EIP_MSR,(val).high,(val).low)

/*
** MSR Feature Control
*/
#define IA32_FEATURE_CONTROL_MSR            0x3aUL

#define IA32_FEATURE_CONTROL_MSR_LOCK_BIT   0
#define IA32_FEATURE_CONTROL_MSR_VMX_BIT    2

#define IA32_FEATURE_CONTROL_MSR_LOCK       (1<<IA32_FEATURE_CONTROL_MSR_LOCK_BIT)
#define IA32_FEATURE_CONTROL_MSR_VMX        (1<<IA32_FEATURE_CONTROL_MSR_VMX_BIT)

typedef union ia32_feature_control_msr
{
   struct
   {
      uint64_t    lock:1;
      uint64_t    unk1:1;
      uint64_t    vmx:1;
      uint64_t    unk2:29;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) feat_ctl_msr_t;

#define rd_msr_feature_ctrl(feat)       rd_msr32(IA32_FEATURE_CONTROL_MSR,(feat).eax)
#define wr_msr_feature_ctrl(feat)       wr_msr32(IA32_FEATURE_CONTROL_MSR,(feat).eax)

/*
** PAT register
*/
#define IA32_PAT_MSR                 0x277

typedef union pat_register
{
   struct
   {
      uint64_t    pa0:3;
      uint64_t    r0:5;
      uint64_t    pa1:3;
      uint64_t    r1:5;
      uint64_t    pa2:3;
      uint64_t    r2:5;
      uint64_t    pa3:3;
      uint64_t    r3:5;
      uint64_t    pa4:3;
      uint64_t    r4:5;
      uint64_t    pa5:3;
      uint64_t    r5:5;
      uint64_t    pa6:3;
      uint64_t    r6:5;
      uint64_t    pa7:3;
      uint64_t    r7:5;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) pat_t;

#define rd_msr_pat(pat)       rd_msr64(IA32_PAT_MSR,(pat).edx,(pat).eax)
#define wr_msr_pat(pat)       wr_msr64(IA32_PAT_MSR,(pat).edx,(pat).eax)

/*
** Functions
*/
#ifdef CONFIG_ARCH_AMD
#include <svm_msr.h>
#else
#include <vmx_msr.h>
#endif

#ifndef __INIT__

#ifdef CONFIG_ARCH_AMD
#include <svm_exit_msr.h>
#else
#include <vmx_exit_msr.h>
#endif

int  __resolve_msr_rd();
int  resolve_msr(uint8_t);

#endif

#endif
