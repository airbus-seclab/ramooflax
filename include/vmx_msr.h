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
#include <msr.h>
#include <vmx_types.h>

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

   msr_t;

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
** IA32_DBG_CTL_MSR
*/
#define IA32_DBG_CTL_MSR                     0x1d9UL

typedef union ia32_debug_ctl_msr
{
   struct
   {
      uint64_t    lbr:1;               /* last branch int/excp */
      uint64_t    btf:1;               /* single step on branch */
      uint64_t    r1:4;                /* reserved (0) */
      uint64_t    tr:1;                /* trace msg enable */
      uint64_t    bts:1;               /* branch trace store */
      uint64_t    btint:1;             /* branch trace interrupt */
      uint64_t    bts_off_os:1;        /* disable branch trace in kernel code */
      uint64_t    bts_off_usr:1;       /* disable branch trace in user code */
      uint64_t    freeze_lbrs_pmi:1;
      uint64_t    freeze_perf_pmi:1;
      uint64_t    r2:1;
      uint64_t    freeze_smm_en:1;     /* disable perf/dbg while in SMM */

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) ia32_dbg_ctl_msr_t;

#define rd_msr_ia32_dbg_ctl(val)      rd_msr64(IA32_DBG_CTL_MSR,(val).edx,(val).eax)

/*
** IA32_PERF_GLOBAL_CTRL_MSR
*/
#define IA32_PERF_GLB_CTL_MSR                 0x38dUL

typedef union ia32_performance_global_control_msr
{
   struct
   {
      uint64_t    pmc0:1;
      uint64_t    pmc1:1;
      uint64_t    r1:30;
      uint64_t    fixed_ctr0:1;
      uint64_t    fixed_ctr1:1;
      uint64_t    fixed_ctr2:1;
      uint64_t    r2:29;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) ia32_perf_glb_ctl_msr_t;

#define rd_msr_ia32_perf_glb_ctl(val)  rd_msr64(IA32_PERF_GLB_CTL_MSR,(val).edx,(val).eax)

/*
** Performance and Debug control capabilities
*/
#define IA32_PERF_CAP_MSR                     0x345UL

#define LBR_FMT_32_OFF                        0
#define LBR_FMT_64_LIN                        1
#define LBR_FMT_64_OFF                        2
#define LBR_FMT_64_FLG                        3

typedef union ia32_performance_capabilities_msr
{
   struct
   {
      uint64_t lbr_fmt:6;
      uint64_t pebs_fmt:1;
      uint64_t pebs_save:1;
      uint64_t pebs_rec_fmt:4;
      uint64_t smm_freeze:1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) ia32_perf_cap_t;

#define rd_msr_ia32_perf_cap(val)  rd_msr64(IA32_PERF_CAP_MSR,(val).edx,(val).eax)

/*
** Last Branch Record MSRs for arch >= Nehalem
*/
#define LBR_TOS_MSR                           0x1c9UL
#define LBR_LER_FROM_LIP_MSR                  0x1ddUL
#define LBR_LER_TO_LIP_MSR                    0x1deUL
#define LBR_FROM_IP_MSR                       0x680UL
#define LBR_TO_IP_MSR                         0x6c0UL

static inline uint64_t __lbr_nehalem(ulong_t idx)
{
   msr_t m;
   rd_msr64(idx, m.edx, m.eax);
   return (m.raw & 0xffffffffffffULL); /* erase sign-ext and mispred */
}

/*
** VMX_BASIC_INFO MSR
*/
#define IA32_VMX_BASIC_MSR                    0x480UL

typedef union vmx_basic_info_msr
{
   struct
   {
      uint64_t     revision_id:32;   /* vmcs revision identifier */
      uint64_t     alloc:13;         /* vmxon, vmcs size */
      uint64_t     r1:3;             /* reserved */
      uint64_t     phys_width:1;     /* 0 on intel64, 1 on non intel64 */
      uint64_t     smm:1;            /* 1 support dual smm */
      uint64_t     mem_type:4;       /* 0 = strong uncacheable, 6 = write-back */
      uint64_t     io_insn:1;        /* 1 ins/outs insn info on vmexit */
      uint64_t     true_f1:1;        /* fixed1 settings may be 0 */
      uint64_t     r2:8;             /* reserved */

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) vmx_basic_info_msr_t;

#define rd_msr_vmx_basic_info(val)  rd_msr64(IA32_VMX_BASIC_MSR,(val).edx,(val).eax)

/*
** VMX fixed bits settings
**
** allow_0 <=> if bit is 1, it is fixed to 1 in destination register (fixed1)
** allow_1 <=> if bit is 0, it is fixed to 0 in destination register (fixed0)
**
** final = (wanted & allow_1) | allow_0
**
*/
#define vmx_set_fixed(_rg,_fx)           ((_rg) = ((_rg) & _fx.allow_1) | _fx.allow_0)

/* XXX: make a macro to create the following
** vmx_xxx_msr types with fields:
** - allowed0
** - allowed1
** - specific bitfields of subsequent type
**   from vmx_vmcs.h (pin, proc, proc2, ...)
*/

/*
** VMX_PINBASED_CTLS MSR
*/
typedef union vmx_pinbased_ctls_msr
{
   struct
   {
      uint32_t  allow_0;
      uint32_t  allow_1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) vmx_pin_ctls_msr_t;

#define rd_msr_vmx_pin_ctls(val)      rd_msr64(0x481UL,(val).edx,(val).eax)
#define rd_msr_vmx_true_pin_ctls(val) rd_msr64(0x48dUL,(val).edx,(val).eax)

/*
** VMX_PROCBASED_CTLS MSR
*/
typedef union vmx_primary_processor_based_ctls_msr
{
   struct
   {
      uint32_t  allow_0;

      union
      {
	 uint32_t allow_1;
	 vmcs_exec_ctl_proc_based_fields_t;

      } __attribute__((packed));

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) vmx_proc_ctls_msr_t;

typedef union vmx_secondary_processor_based_ctls_msr
{
   struct
   {
      uint32_t  allow_0;

      union
      {
	 uint32_t allow_1;
	 vmcs_exec_ctl_proc2_based_fields_t;

      } __attribute__((packed));

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) vmx_proc2_ctls_msr_t;


#define rd_msr_vmx_proc_ctls(val)      rd_msr64(0x482UL,(val).edx,(val).eax)
#define rd_msr_vmx_proc2_ctls(val)     rd_msr64(0x48bUL,(val).edx,(val).eax)
#define rd_msr_vmx_true_proc_ctls(val) rd_msr64(0x48eUL,(val).edx,(val).eax)

/*
** VMX_EXIT_CTLS MSR
*/
typedef union vmx_exit_ctls_msr
{
   struct
   {
      uint32_t  allow_0;
      uint32_t  allow_1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) vmx_exit_ctls_msr_t;

#define rd_msr_vmx_exit_ctls(val)      rd_msr64(0x483UL,(val).edx,(val).eax)
#define rd_msr_vmx_true_exit_ctls(val) rd_msr64(0x48fUL,(val).edx,(val).eax)


/*
** VMX_ENTRY_CTLS MSR
*/
typedef union vmx_entry_ctls_msr
{
   struct
   {
      uint32_t  allow_0;
      uint32_t  allow_1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) vmx_entry_ctls_msr_t;

#define rd_msr_vmx_entry_ctls(val)      rd_msr64(0x484UL,(val).edx,(val).eax)
#define rd_msr_vmx_true_entry_ctls(val) rd_msr64(0x490UL,(val).edx,(val).eax)


/*
** VMX_MISC_DATA MSR
*/
typedef union vmx_misc_data_msr
{
   struct
   {
      uint64_t   preempt_rate:5; /* 0-4   cf. appendix G.6 */
      uint64_t   lma:1;          /* 5     efer.lma stored in ia32e guest on exit */
      uint64_t   hlt:1;          /* 6     support for hlt */
      uint64_t   sht:1;          /* 7     support for shutdown */
      uint64_t   ipi:1;          /* 8     support for SIPI */
      uint64_t   r2:7;           /* 9-15  reserved */
      uint64_t   cr3:9;          /* 16-24 nr of cr3 target values 0 - 256 (0x100) */
      uint64_t   msr:3;          /* 25-27 maximum nr of msr store list */
      uint64_t   smm:1;          /* 28    smm monitor ctl */
      uint64_t   r3:4;           /* 29-31 reserved */
      uint64_t   mseg:32;        /* 32-63 MSEG revision identifier */

   } __attribute__((packed));

   msr_t;

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

   msr_t;

} __attribute__((packed)) vmx_vmcs_enum_msr_t;

#define rd_msr_vmx_vmcs_enum(val)      rd_msr64(0x48aUL,(val).edx,(val).eax)

/*
** Control registers MSRs fixed bits
*/
typedef union vmx_cr_ctls_msr
{
   struct
   {
      uint32_t  allow_0;
      uint32_t  allow_1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) vmx_fixed_cr_msr_t;

#define rd_msr_vmx_fixed_cr0(val)	\
   ({					\
      rd_msr32(0x486UL,(val).allow_0);	\
      rd_msr32(0x487UL,(val).allow_1);	\
   })

#define vmx_allow_cr0_pe_disable(_fx)  (!(_fx.allow_0 & (1<<0)))
#define vmx_allow_cr0_pg_disable(_fx)  (!(_fx.allow_0 & (1<<31)))

#define rd_msr_vmx_fixed_cr4(val)	\
   ({					\
      rd_msr32(0x488UL,(val).allow_0);	\
      rd_msr32(0x489UL,(val).allow_1);	\
   })

/*
** VPID & EPT capabilities MSRs
*/
typedef union vmx_ept_vpid_cap_msr
{
   struct
   {
      uint64_t  xo:1;         /* 0     allow execute only ept entry */
      uint64_t  r1:5;         /* 1-5   reserved */
      uint64_t  pwl4:1;       /* 6     page walk length of 4 */
      uint64_t  r2:1;         /* 7     reserved */
      uint64_t  uc:1;         /* 8     allow UC for ept structs */
      uint64_t  r3:5;         /* 9-13  reserved */
      uint64_t  wb:1;         /* 14    allow WB for ept structs */
      uint64_t  r4:1;         /* 15    reserved */
      uint64_t  pg_2m:1;      /* 16    allow ept pde to map 2MB pages */
      uint64_t  pg_1g:1;      /* 17    allow ept pdpte to map 1GB page */
      uint64_t  r5:2;         /* 18-19 reserved */
      uint64_t  invept:1;     /* 20    invept insn supported */
      uint64_t  r6:4;         /* 21-24 reserved */
      uint64_t  invept_s:1;   /* 25    single context invept */
      uint64_t  invept_a:1;   /* 26    all context invept */
      uint64_t  r7:5;         /* 27-31 reserved */
      uint64_t  invvpid:1;    /* 32    invvpid insn supported */
      uint64_t  r8:7;         /* 33-39 reserved */
      uint64_t  invvpid_i:1;  /* 40    individual invvpid */
      uint64_t  invvpid_s:1;  /* 41    single context invvpid */
      uint64_t  invvpid_a:1;  /* 42    all context invvpid */
      uint64_t  invvpid_r:1;  /* 43    single context retaining globals invvpid */
      uint64_t  r9:20;        /* 44-63 reserved */

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) vmx_ept_cap_msr_t;

#define rd_msr_vmx_ept_cap(val)        rd_msr64(0x48cUL,(val).edx,(val).eax)


#endif
