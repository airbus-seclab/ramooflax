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
#ifndef __VMX_MSR_H__
#define __VMX_MSR_H__

#include <types.h>

/*
** IA32_EFER_MSR
*/
#define IA32_EFER_MSR          0xc0000080UL

#define IA32_EFER_LME_BIT       8
#define IA32_EFER_LMA_BIT      10
#define IA32_EFER_NXE_BIT      11

#define IA32_EFER_LME          (1<<IA32_EFER_LME_BIT)
#define IA32_EFER_LMA          (1<<IA32_EFER_LMA_BIT)
#define IA32_EFER_NXE          (1<<IA32_EFER_NXE_BIT)

typedef union ia32_efer_msr
{
   struct
   {
      uint64_t    sce:1;     /* syscall extensions */
      uint64_t    rsrvd0:7;  /* zero */
      uint64_t    lme:1;     /* long mode enable */
      uint64_t    rsrvd1:1;  /* zero */
      uint64_t    lma:1;     /* long mode active */
      uint64_t    nxe:1;     /* no-execute-enable */

   } __attribute__((packed));

   raw_msr_entry_t;

} __attribute__((packed)) ia32_efer_msr_t;

#define rd_msr_ia32_efer(val)      rd_msr64(IA32_EFER_MSR,(val).edx,(val).eax)
#define wr_msr_ia32_efer(val)      wr_msr64(IA32_EFER_MSR,(val).edx,(val).eax)

#define set_ia32_efer(_n,_v)			\
   ({						\
      ia32_efer_msr_t efer;			\
      rd_msr_ia32_efer(efer);			\
      if(_v) efer.raw |=  (1ULL<<(_n));		\
      else   efer.raw &= ~(1ULL<<(_n));		\
      wr_msr_ia32_efer(efer);			\
   })

#define lm_enable()           set_ia32_efer(IA32_EFER_LME_BIT,1)
#define lm_disable()          set_ia32_efer(IA32_EFER_LME_BIT,0)
#define lm_active()           ({ia32_efer_msr_t m; rd_msr_ia32_efer(m); m.lma;})

/*
** IA32_DEBUGCTL_MSR
*/
#define IA32_DBGCTL_MSR                     0x1d9UL

typedef union ia32_debugctl_msr
{
   raw_msr_entry_t;

   struct
   {
      uint64_t    lbr:1;    /* last branch int/excp */
      uint64_t    btf:1;    /* single step on branch */
      uint64_t    r1:4;     /* reserved (0) */
      uint64_t    tr:1;     /* trace msg enable */
      uint64_t    bts:1;    /* branch trace store */
      uint64_t    btint:1;  /* branch trace interrupt */
      uint64_t    r2:23;    /* reserved (0) */

   } __attribute__((packed));

} __attribute__((packed)) ia32_debugctl_msr_t;

#define rd_msr_ia32_debugctl(val)      rd_msr64(IA32_DBGCTL_MSR,(val).edx,(val).eax)


/*
** VMX_BASIC_INFO MSR
*/
typedef union vmx_basic_info_msr
{
   struct
   {
      uint64_t     revision_id:32;   /* vmcs revision identifier */
      uint64_t     alloc:13;         /* vmxon, vmcs size */
      uint64_t     r1:3;             /* reserved */
      uint64_t     phys_width:1;     /* 1 on intel64, 0 on non intel64 */
      uint64_t     smm:1;            /* always 1 */
      uint64_t     mem_type:4;       /* 0 = strong uncacheable, 6 = write-back */
      uint64_t     r2:10;            /* reserved */

   } __attribute__((packed));

   raw_msr_entry_t;

} __attribute__((packed)) vmx_basic_info_msr_t;

#define rd_msr_vmx_basic_info(val)      rd_msr64(0x480UL,(val).edx,(val).eax)

/*
** VMX_PINBASED_CTLS MSR
**
** allowed_0: fixed bits to 1
** allowed_1: fixed bits to 0
**
** result (undefined cases removed) : (my_pin & allowed1) | allowed0
*/
typedef union vmx_pinbased_ctls_msr
{
   struct
   {
      uint32_t  allowed_0_settings;
      uint32_t  allowed_1_settings;

   } __attribute__((packed));

   raw_msr_entry_t;

} __attribute__((packed)) vmx_pinbased_ctls_msr_t;

#define rd_msr_vmx_pinbased_ctls(val)      rd_msr64(0x481UL,(val).edx,(val).eax)

/*
** VMX_PROCBASED_CTLS MSR
*/
typedef union vmx_procbased_ctls_msr
{
   struct
   {
      uint32_t  allowed_0_settings;
      uint32_t  allowed_1_settings;

   } __attribute__((packed));

   raw_msr_entry_t;

} __attribute__((packed)) vmx_procbased_ctls_msr_t;

#define rd_msr_vmx_procbased_ctls(val)      rd_msr64(0x482UL,(val).edx,(val).eax)

/*
** VMX_EXIT_CTLS MSR
*/
typedef union vmx_exit_ctls_msr
{
   struct
   {
      uint32_t  allowed_0_settings;
      uint32_t  allowed_1_settings;

   } __attribute__((packed));

   raw_msr_entry_t;

} __attribute__((packed)) vmx_exit_ctls_msr_t;

#define rd_msr_vmx_exit_ctls(val)      rd_msr64(0x483UL,(val).edx,(val).eax)


/*
** VMX_ENTRY_CTLS MSR
*/
typedef union vmx_entry_ctls_msr
{
   struct
   {
      uint32_t  allowed_0_settings;
      uint32_t  allowed_1_settings;

   } __attribute__((packed));

   raw_msr_entry_t;

} __attribute__((packed)) vmx_entry_ctls_msr_t;

#define rd_msr_vmx_entry_ctls(val)      rd_msr64(0x484UL,(val).edx,(val).eax)


/*
** VMX_MISC_DATA MSR
*/
typedef union vmx_misc_data_msr
{
   struct
   {
      uint64_t   r1:6;   /* 0-5   reserved */
      uint64_t   hlt:1;  /* 6     support for hlt */
      uint64_t   sht:1;  /* 7     support for shutdown */
      uint64_t   ipi:1;  /* 8     support for SIPI */
      uint64_t   r2:7;   /* 9-15  reserved */
      uint64_t   cr3:9;  /* 16-24 number of cr3 target values 0 - 256 (0x100) */
      uint64_t   msr:3;  /* 25-27 maximum nr of msr store list */
      uint64_t   r3:4;   /* 28-31 reserved */
      uint64_t   mseg:32;   /* 32-63 MSEG revision identifier */

   } __attribute__((packed));

   raw_msr_entry_t;

} __attribute__((packed)) vmx_misc_data_msr_t;

#define rd_msr_vmx_misc_data(val)      rd_msr64(0x485UL,(val).edx,(val).eax)

/*
** VMX_VMCS_ENUM MSR
*/
typedef union vmx_vmcs_enum_msr
{
   struct
   {
      uint64_t  r1:1;     /* 0     reserved */
      uint64_t  index:9;  /* 1-9   highest index value used for any vmcs encoding */
      uint64_t  r2:22;    /* 10-31 reserved */
      uint64_t  r3:32;    /* 32-63 reserved */

   } __attribute__((packed));

   raw_msr_entry_t;

} __attribute__((packed)) vmx_vmcs_enum_msr_t;

#define rd_msr_vmx_vmcs_enum(val)      rd_msr64(0x48aUL,(val).edx,(val).eax)

/*
** Control registers MSRs
**
** CR0 and CR4 fixed bits
*/
#define rd_msr_vmx_cr0_fixed0(val)      rd_msr32(0x486UL,(val).low)
#define rd_msr_vmx_cr0_fixed1(val)      rd_msr32(0x487UL,(val).low)

#define rd_msr_vmx_cr4_fixed0(val)      rd_msr32(0x488UL,(val).low)
#define rd_msr_vmx_cr4_fixed1(val)      rd_msr32(0x489UL,(val).low)



#endif
