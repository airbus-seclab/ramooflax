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

#define CPUID_ECX_FEAT_MWAIT_BIT        3
#define CPUID_ECX_FEAT_PERF_CAP_BIT    15

#define CPUID_ECX_FEAT_MWAIT            (1<<CPUID_ECX_FEAT_MWAIT_BIT)
#define CPUID_ECX_FEAT_PERF_CAP         (1<<CPUID_ECX_FEAT_PERF_CAP_BIT)

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

#define vme_supported()				\
   ({						\
      uint32_t c=0,d;				\
      cpuid_features(c,d);			\
      (d & CPUID_EDX_FEAT_VME)?1:0;		\
   })

#define apic_supported()			\
   ({						\
      uint32_t c=0,d;				\
      cpuid_features(c,d);			\
      (d & CPUID_EDX_FEAT_APIC)?1:0;		\
   })

#define perf_cap_supported()			\
   ({						\
      uint32_t c,d=0;				\
      cpuid_features(c,d);			\
      (c & CPUID_ECX_FEAT_PERF_CAP)?1:0;	\
   })

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
** Functions
*/
#ifdef __SVM__
#include <svm_cpuid.h>
#else
#include <vmx_cpuid.h>
#endif

#ifndef __INIT__

#define CPUID_FAIL     0
#define CPUID_SUCCESS  1

#ifdef __SVM__
#include <svm_exit_cpuid.h>
#else
#include <vmx_exit_cpuid.h>
#endif

int    resolve_cpuid();
#endif

#endif
