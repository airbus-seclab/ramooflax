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
#ifndef __CPUID_H__
#define __CPUID_H__

#include <types.h>
#include <config.h>

/*
** CPUID
*/
#define __cpuid_1(idx,eax)						\
   asm volatile ("cpuid":"=a"(eax):"a"(idx):"ebx","ecx","edx")

#define __cpuid_2(idx,ecx,edx)						\
   asm volatile ("cpuid":"=c"(ecx),"=d"(edx):"a"(idx),"c"(ecx):"ebx")

#define __cpuid_4_no_pic(idx,eax,ebx,ecx,edx)				\
   asm volatile ("cpuid":"=a"(eax),"=b"(ebx),"=c"(ecx),"=d"(edx):"a"(idx),"c"(ecx))

/*
** we preserve ebx and ecx used by
** gcc as base register for 32 bits PIE/PIC
*/
#ifndef __X86_64__
#define __cpuid_4_pic(idx,eax,ebx,ecx,edx)				\
   asm volatile (							\
      "xchg  %%esi, %%ecx  \n"						\
      "push  %%ebx         \n"						\
      "cpuid               \n"						\
      "movl  %%ebx, %1     \n"						\
      "popl  %%ebx         \n"						\
      "xchg  %%esi, %%ecx  \n"						\
      :"=a"(eax),"=r"(ebx),"=S"(ecx),"=d"(edx)				\
      :"a"(idx),"S"(ecx)						\
      :"cc" )
#else
/*
** we use small pie model and so no GOT is used,
** every symbol is 'rip relative' accessed
*/
#define __cpuid_4_pic(i,a,b,c,d)                __cpuid_4_no_pic(i,a,b,c,d)
#endif

#ifdef __INIT__
#define __cpuid_4(i,a,b,c,d)                    __cpuid_4_no_pic(i,a,b,c,d)
#else
#define __cpuid_4(i,a,b,c,d)                    __cpuid_4_pic(i,a,b,c,d)
#endif

#define __cpuid(a,b,c,d)                        __cpuid_4(a,a,b,c,d)

/*
** CPUID Features & Extended Features
*/
#define CPUID_FEATURE_INFO               1

typedef union cpuid_feature_info_eax
{
   struct
   {
      uint32_t  stepping_id:4;
      uint32_t  model:4;
      uint32_t  family:4;
      uint32_t  type:2;
      uint32_t  rsvd0:2;
      uint32_t  ext_model:4;
      uint32_t  ext_family:8;
      uint32_t  rsvd1:4;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) cpuid_feat_info_eax_t;

typedef union cpuid_feature_info_ebx
{
   struct
   {
      uint32_t   brand_id:8;
      uint32_t   clflush_sz:8;
      uint32_t   logical_cpu_nr:8;
      uint32_t   lapic_id:8;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) cpuid_feat_info_ebx_t;

#define CPUID_ECX_FEAT_SSE3_BIT         0
#define CPUID_ECX_FEAT_PCLMULQDQ_BIT    1
#define CPUID_ECX_FEAT_DTES64_BIT       2
#define CPUID_ECX_FEAT_MONITOR_BIT      3
#define CPUID_ECX_FEAT_DSCPL_BIT        4
#define CPUID_ECX_FEAT_VMX_BIT          5
#define CPUID_ECX_FEAT_SMX_BIT          6
#define CPUID_ECX_FEAT_EIST_BIT         7
#define CPUID_ECX_FEAT_TM2_BIT          8
#define CPUID_ECX_FEAT_SSSE3_BIT        9
#define CPUID_ECX_FEAT_CNXT_BIT        10
#define CPUID_ECX_FEAT_FMA_BIT         12
#define CPUID_ECX_FEAT_CMPXCHG16B_BIT  13
#define CPUID_ECX_FEAT_xTPR_BIT        14
#define CPUID_ECX_FEAT_PDCM_BIT        15
#define CPUID_ECX_FEAT_PCID_BIT        17
#define CPUID_ECX_FEAT_DCA_BIT         18
#define CPUID_ECX_FEAT_SSE41_BIT       19
#define CPUID_ECX_FEAT_SSE42_BIT       20
#define CPUID_ECX_FEAT_x2APIC_BIT      21
#define CPUID_ECX_FEAT_MOVBE_BIT       22
#define CPUID_ECX_FEAT_POPCNT_BIT      23
#define CPUID_ECX_FEAT_TSCD_BIT        24
#define CPUID_ECX_FEAT_AESNI_BIT       25
#define CPUID_ECX_FEAT_XSAVE_BIT       26
#define CPUID_ECX_FEAT_OSXSAVE_BIT     27
#define CPUID_ECX_FEAT_AVX_BIT         28
#define CPUID_ECX_FEAT_RDRAND_BIT      30

#define CPUID_ECX_FEAT_SSE3        (1<<CPUID_ECX_FEAT_SSE3_BIT)
#define CPUID_ECX_FEAT_PCLMULQDQ   (1<<CPUID_ECX_FEAT_PCLMULQDQ_BIT)
#define CPUID_ECX_FEAT_DTES64      (1<<CPUID_ECX_FEAT_DTES64_BIT)
#define CPUID_ECX_FEAT_MONITOR     (1<<CPUID_ECX_FEAT_MONITOR_BIT)
#define CPUID_ECX_FEAT_DSCPL       (1<<CPUID_ECX_FEAT_DSCPL_BIT)
#define CPUID_ECX_FEAT_VMX         (1<<CPUID_ECX_FEAT_VMX_BIT)
#define CPUID_ECX_FEAT_SMX         (1<<CPUID_ECX_FEAT_SMX_BIT)
#define CPUID_ECX_FEAT_EIST        (1<<CPUID_ECX_FEAT_EIST_BIT)
#define CPUID_ECX_FEAT_TM2         (1<<CPUID_ECX_FEAT_TM2_BIT)
#define CPUID_ECX_FEAT_SSSE3       (1<<CPUID_ECX_FEAT_SSSE3_BIT)
#define CPUID_ECX_FEAT_CNXT        (1<<CPUID_ECX_FEAT_CNXT_BIT)
#define CPUID_ECX_FEAT_FMA         (1<<CPUID_ECX_FEAT_FMA_BIT)
#define CPUID_ECX_FEAT_CMPXCHG16B  (1<<CPUID_ECX_FEAT_CMPXCHG16B_BIT)
#define CPUID_ECX_FEAT_xTPR        (1<<CPUID_ECX_FEAT_xTPR_BIT)
#define CPUID_ECX_FEAT_PDCM        (1<<CPUID_ECX_FEAT_PDCM_BIT)
#define CPUID_ECX_FEAT_PCID        (1<<CPUID_ECX_FEAT_PCID_BIT)
#define CPUID_ECX_FEAT_DCA         (1<<CPUID_ECX_FEAT_DCA_BIT)
#define CPUID_ECX_FEAT_SSE41       (1<<CPUID_ECX_FEAT_SSE41_BIT)
#define CPUID_ECX_FEAT_SSE42       (1<<CPUID_ECX_FEAT_SSE42_BIT)
#define CPUID_ECX_FEAT_x2APIC      (1<<CPUID_ECX_FEAT_x2APIC_BIT)
#define CPUID_ECX_FEAT_MOVBE       (1<<CPUID_ECX_FEAT_MOVBE_BIT)
#define CPUID_ECX_FEAT_POPCNT      (1<<CPUID_ECX_FEAT_POPCNT_BIT)
#define CPUID_ECX_FEAT_TSCD        (1<<CPUID_ECX_FEAT_TSCD_BIT)
#define CPUID_ECX_FEAT_AESNI       (1<<CPUID_ECX_FEAT_AESNI_BIT)
#define CPUID_ECX_FEAT_XSAVE       (1<<CPUID_ECX_FEAT_XSAVE_BIT)
#define CPUID_ECX_FEAT_OSXSAVE     (1<<CPUID_ECX_FEAT_OSXSAVE_BIT)
#define CPUID_ECX_FEAT_AVX         (1<<CPUID_ECX_FEAT_AVX_BIT)
#define CPUID_ECX_FEAT_RDRAND      (1<<CPUID_ECX_FEAT_RDRAND_BIT)

#define CPUID_EDX_FEAT_FPU_BIT          0
#define CPUID_EDX_FEAT_VME_BIT          1
#define CPUID_EDX_FEAT_DE_BIT           2
#define CPUID_EDX_FEAT_PSE_BIT          3
#define CPUID_EDX_FEAT_TSC_BIT          4
#define CPUID_EDX_FEAT_MSR_BIT          5
#define CPUID_EDX_FEAT_PAE_BIT          6
#define CPUID_EDX_FEAT_MCE_BIT          7
#define CPUID_EDX_FEAT_CX8_BIT          8
#define CPUID_EDX_FEAT_APIC_BIT         9
#define CPUID_EDX_FEAT_SEP_BIT         11
#define CPUID_EDX_FEAT_MTRR_BIT        12
#define CPUID_EDX_FEAT_PGE_BIT         13
#define CPUID_EDX_FEAT_MCA_BIT         14
#define CPUID_EDX_FEAT_CMOV_BIT        15
#define CPUID_EDX_FEAT_PAT_BIT         16
#define CPUID_EDX_FEAT_PSE36_BIT       17
#define CPUID_EDX_FEAT_PSN_BIT         18
#define CPUID_EDX_FEAT_CLFSH_BIT       19
#define CPUID_EDX_FEAT_DS_BIT          21
#define CPUID_EDX_FEAT_ACPI_BIT        22
#define CPUID_EDX_FEAT_MMX_BIT         23
#define CPUID_EDX_FEAT_FXSR_BIT        24
#define CPUID_EDX_FEAT_SSE_BIT         25
#define CPUID_EDX_FEAT_SSE2_BIT        26
#define CPUID_EDX_FEAT_SS_BIT          27
#define CPUID_EDX_FEAT_HTT_BIT         28
#define CPUID_EDX_FEAT_TM_BIT          29
#define CPUID_EDX_FEAT_PBE_BIT         31

#define CPUID_EDX_FEAT_FPU             (1<<CPUID_EDX_FEAT_FPU_BIT)
#define CPUID_EDX_FEAT_VME             (1<<CPUID_EDX_FEAT_VME_BIT)
#define CPUID_EDX_FEAT_DE              (1<<CPUID_EDX_FEAT_DE_BIT)
#define CPUID_EDX_FEAT_PSE             (1<<CPUID_EDX_FEAT_PSE_BIT)
#define CPUID_EDX_FEAT_TSC             (1<<CPUID_EDX_FEAT_TSC_BIT)
#define CPUID_EDX_FEAT_MSR             (1<<CPUID_EDX_FEAT_MSR_BIT)
#define CPUID_EDX_FEAT_PAE             (1<<CPUID_EDX_FEAT_PAE_BIT)
#define CPUID_EDX_FEAT_MCE             (1<<CPUID_EDX_FEAT_MCE_BIT)
#define CPUID_EDX_FEAT_CX8             (1<<CPUID_EDX_FEAT_CX8_BIT)
#define CPUID_EDX_FEAT_APIC            (1<<CPUID_EDX_FEAT_APIC_BIT)
#define CPUID_EDX_FEAT_SEP             (1<<CPUID_EDX_FEAT_SEP_BIT)
#define CPUID_EDX_FEAT_MTRR            (1<<CPUID_EDX_FEAT_MTRR_BIT)
#define CPUID_EDX_FEAT_PGE             (1<<CPUID_EDX_FEAT_PGE_BIT)
#define CPUID_EDX_FEAT_MCA             (1<<CPUID_EDX_FEAT_MCA_BIT)
#define CPUID_EDX_FEAT_CMOV            (1<<CPUID_EDX_FEAT_CMOV_BIT)
#define CPUID_EDX_FEAT_PAT             (1<<CPUID_EDX_FEAT_PAT_BIT)
#define CPUID_EDX_FEAT_PSE36           (1<<CPUID_EDX_FEAT_PSE36_BIT)
#define CPUID_EDX_FEAT_PSN             (1<<CPUID_EDX_FEAT_PSN_BIT)
#define CPUID_EDX_FEAT_CLFSH           (1<<CPUID_EDX_FEAT_CLFSH_BIT)
#define CPUID_EDX_FEAT_DS              (1<<CPUID_EDX_FEAT_DS_BIT)
#define CPUID_EDX_FEAT_ACPI            (1<<CPUID_EDX_FEAT_ACPI_BIT)
#define CPUID_EDX_FEAT_MMX             (1<<CPUID_EDX_FEAT_MMX_BIT)
#define CPUID_EDX_FEAT_FXSR            (1<<CPUID_EDX_FEAT_FXSR_BIT)
#define CPUID_EDX_FEAT_SSE             (1<<CPUID_EDX_FEAT_SSE_BIT)
#define CPUID_EDX_FEAT_SSE2            (1<<CPUID_EDX_FEAT_SSE2_BIT)
#define CPUID_EDX_FEAT_SS              (1<<CPUID_EDX_FEAT_SS_BIT)
#define CPUID_EDX_FEAT_HTT             (1<<CPUID_EDX_FEAT_HTT_BIT)
#define CPUID_EDX_FEAT_TM              (1<<CPUID_EDX_FEAT_TM_BIT)
#define CPUID_EDX_FEAT_PBE             (1<<CPUID_EDX_FEAT_PBE_BIT)

#define cpuid_features(eCx,eDx)        __cpuid_2(CPUID_FEATURE_INFO,eCx,eDx)

#define cpuid_has_c_feat(feAt)	\
   ({		       		\
      uint32_t c=0,d;		\
      cpuid_features(c,d);	\
      (c & (feAt))?1:0;		\
   })

#define cpuid_has_d_feat(feAt)	\
   ({				\
      uint32_t c,d=0;		\
      cpuid_features(c,d);	\
      (d & (feAt))?1:0;		\
   })

#define perf_cap_supported()  cpuid_has_c_feat(CPUID_ECX_FEAT_PDCM)
#define osxsave_supported()   cpuid_has_c_feat(CPUID_ECX_FEAT_XSAVE)
#define vme_supported()       cpuid_has_d_feat(CPUID_EDX_FEAT_VME)
#define apic_supported()      cpuid_has_d_feat(CPUID_EDX_FEAT_APIC)

/*
** CPUID Maximum supported extension
*/
#define CPUID_MAX_EXT                   0x80000000
#define cpuid_max_ext()			\
   ({					\
      uint32_t eax;			\
      __cpuid_1(CPUID_MAX_EXT,eax);	\
      eax;				\
   })

/*
** Various CPUID
*/
#define CPUID_BASIC_INFO               0
#define CPUID_PROC_SERIAL              3
#define CPUID_BRAND_STR1               0x80000002
#define CPUID_BRAND_STR2               0x80000003
#define CPUID_BRAND_STR3               0x80000004

/*
** CPUID Maximum physical/linear supported width
** vmx/svm dependent
*/
#define CPUID_MAX_ADDR                  0x80000008
#define cpuid_max_addr(_x)              __cpuid_1(CPUID_MAX_ADDR, (_x))

/*
** CPUID Extended Processor Features
*/
#define CPUID_EXT_PROC_FEAT                 0x80000001

#define CPUID_EDX_EXT_PROC_FEAT_PSE_BIT     3
#define CPUID_EDX_EXT_PROC_FEAT_PAE_BIT     6
#define CPUID_EDX_EXT_PROC_FEAT_PGE_BIT    13
#define CPUID_EDX_EXT_PROC_FEAT_PAT_BIT    16
#define CPUID_EDX_EXT_PROC_FEAT_NX_BIT     20
#define CPUID_EDX_EXT_PROC_FEAT_P1G_BIT    26
#define CPUID_EDX_EXT_PROC_FEAT_LM_BIT     29

#define CPUID_EDX_EXT_PROC_FEAT_PSE        (1<<CPUID_EDX_EXT_PROC_FEAT_PSE_BIT)
#define CPUID_EDX_EXT_PROC_FEAT_PAE        (1<<CPUID_EDX_EXT_PROC_FEAT_PAE_BIT)
#define CPUID_EDX_EXT_PROC_FEAT_PGE        (1<<CPUID_EDX_EXT_PROC_FEAT_PGE_BIT)
#define CPUID_EDX_EXT_PROC_FEAT_PAT        (1<<CPUID_EDX_EXT_PROC_FEAT_PAT_BIT)
#define CPUID_EDX_EXT_PROC_FEAT_NX         (1<<CPUID_EDX_EXT_PROC_FEAT_NX_BIT)
#define CPUID_EDX_EXT_PROC_FEAT_P1G        (1<<CPUID_EDX_EXT_PROC_FEAT_P1G_BIT)
#define CPUID_EDX_EXT_PROC_FEAT_LM         (1<<CPUID_EDX_EXT_PROC_FEAT_LM_BIT)

#define cpuid_ext_proc_feat(eCx,eDx)       __cpuid_2(CPUID_EXT_PROC_FEAT,eCx,eDx)

#define page_1G_supported()			\
   ({						\
      uint32_t c=0,d;				\
      cpuid_ext_proc_feat(c,d);			\
      (d & CPUID_EDX_EXT_PROC_FEAT_P1G)?1:0;	\
   })


/*
** MS Hyper-V leaf
*/
#define CPUID_MSHYPERV_RANGE                0x40000000
#define CPUID_MSHYPERV_ID                   0x40000001
#define CPUID_MSHYPERV_FEAT                 0x40000003

#define CPUID_MSHYPERV_SIG                  0x31237648

typedef union cpuid_ms_hyperv_feature_info_eax
{
   struct
   {
      uint32_t  vprt:1;
      uint32_t  time_ref_cnt:1;
      uint32_t  synic:1;
      uint32_t  stimer:1;
      uint32_t  apic:1;
      uint32_t  hc:1;
      uint32_t  vpidx:1;
      uint32_t  vprst:1;
      uint32_t  stat:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) mshyperv_feat_eax_t;

typedef union cpuid_ms_hyperv_feature_info_edx
{
   struct
   {
      uint32_t  mwait:1;
      uint32_t  gdbg:1;
      uint32_t  perf:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) mshyperv_feat_edx_t;

/*
** Functions
*/
#ifdef CONFIG_ARCH_AMD
#include <svm_cpuid.h>
#else
#include <vmx_cpuid.h>
#endif

#ifndef __INIT__

#ifdef CONFIG_ARCH_AMD
#include <svm_exit_cpuid.h>
#else
#include <vmx_exit_cpuid.h>
#endif

int    resolve_cpuid();
#endif

#endif
