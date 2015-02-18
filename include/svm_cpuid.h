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
#ifndef __SVM_CPUID_H__
#define __SVM_CPUID_H__

#include <types.h>

/*
** CPUID Extended Processor Features
*/
#define CPUID_AMD_ECX_EXT_PROC_FEAT_CMP_BIT   1
#define CPUID_AMD_ECX_EXT_PROC_FEAT_SVM_BIT   2
#define CPUID_AMD_ECX_EXT_PROC_FEAT_CMP       (1<<CPUID_AMD_ECX_EXT_PROC_FEAT_CMP_BIT)
#define CPUID_AMD_ECX_EXT_PROC_FEAT_SVM       (1<<CPUID_AMD_ECX_EXT_PROC_FEAT_SVM_BIT)

#define CPUID_AMD_EDX_EXT_PROC_FEAT_PSE_BIT   3
#define CPUID_AMD_EDX_EXT_PROC_FEAT_PAE_BIT   6
#define CPUID_AMD_EDX_EXT_PROC_FEAT_PGE_BIT  13
#define CPUID_AMD_EDX_EXT_PROC_FEAT_PAT_BIT  16

#define svm_supported()					\
   ({							\
      uint32_t c=0,d;					\
      cpuid_ext_proc_feat(c,d);				\
      (c & CPUID_AMD_ECX_EXT_PROC_FEAT_SVM)?1:0;	\
   })

#define check_cpu_skillz()						\
   ({									\
      uint32_t c=0,d;							\
      uint32_t efeat =							\
	 CPUID_EDX_EXT_PROC_FEAT_PSE |					\
	 CPUID_EDX_EXT_PROC_FEAT_PAE |					\
	 CPUID_EDX_EXT_PROC_FEAT_PAT |					\
	 CPUID_EDX_EXT_PROC_FEAT_LM;					\
      cpuid_ext_proc_feat(c,d);						\
      if((d & efeat) != efeat)						\
	 panic("cpu not skilled enough: pse %d pae %d pat %d lm %d",	\
	       (d&CPUID_EDX_EXT_PROC_FEAT_PSE)?1:0,			\
	       (d&CPUID_EDX_EXT_PROC_FEAT_PAE)?1:0,			\
	       (d&CPUID_EDX_EXT_PROC_FEAT_PAT)?1:0,			\
	       (d&CPUID_EDX_EXT_PROC_FEAT_LM)?1:0			\
	    );								\
   })

/*
** CPUID_MAX_ADDR
**
** Gives Addr Size and Physical Core count
*/
#define CPUID_AMD_PHYS_CORE    CPUID_MAX_ADDR

/* EAX */
typedef union amd_max_addr_sz
{
   struct
   {
      uint32_t  paddr_sz:8;
      uint32_t  vaddr_sz:8;
      uint32_t  g_paddr_sz:8;
      uint32_t  rsrvd0:8;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) amd_max_addr_sz_t;

/* ECX */
typedef union amd_max_core
{
   struct
   {
      uint32_t   nc:8;
      uint32_t   rsrvd2:4;
      uint32_t   apic_id_core_id_sz:4;
      uint32_t   rsrvd3:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) amd_max_core_t;

/*
** AMD CPUID SVM features
*/
#define CPUID_AMD_SVM_FEATURES                    0x8000000a

#define CPUID_AMD_SVM_NPT_FEATURE_BIT                      0
#define CPUID_AMD_SVM_LBR_FEATURE_BIT                      1
#define CPUID_AMD_SVM_LOCK_FEATURE_BIT                     2
#define CPUID_AMD_SVM_NRIP_FEATURE_BIT                     3
#define CPUID_AMD_SVM_TSCRATE_FEATURE_BIT                  4
#define CPUID_AMD_SVM_VMCB_CLEAN_FEATURE_BIT               5
#define CPUID_AMD_SVM_FLUSH_BY_ASID_FEATURE_BIT            6
#define CPUID_AMD_SVM_DECODE_ASSIST_FEATURE_BIT            7
#define CPUID_AMD_SVM_PAUSE_FILTER_FEATURE_BIT            10
#define CPUID_AMD_SVM_PAUSE_THRESH_FEATURE_BIT            12

#define CPUID_AMD_SVM_NPT_FEATURE           (1<<CPUID_AMD_SVM_NPT_FEATURE_BIT)
#define CPUID_AMD_SVM_LBR_FEATURE           (1<<CPUID_AMD_SVM_LBR_FEATURE_BIT)
#define CPUID_AMD_SVM_LOCK_FEATURE          (1<<CPUID_AMD_SVM_LOCK_FEATURE_BIT)
#define CPUID_AMD_SVM_NRIP_FEATURE          (1<<CPUID_AMD_SVM_NRIP_FEATURE_BIT)
#define CPUID_AMD_SVM_TSCRATE_FEATURE       (1<<CPUID_AMD_SVM_TSCRATE_FEATURE_BIT)
#define CPUID_AMD_SVM_VMCB_CLEAN_FEATURE    (1<<CPUID_AMD_SVM_VMCB_CLEAN_FEATURE_BIT)
#define CPUID_AMD_SVM_FLUSH_BY_ASID_FEATURE (1<<CPUID_AMD_SVM_FLUSH_BY_ASID_FEATURE_BIT)
#define CPUID_AMD_SVM_DECODE_ASSIST_FEATURE (1<<CPUID_AMD_SVM_DECODE_ASSIST_FEATURE_BIT)
#define CPUID_AMD_SVM_PAUSE_FILTER_FEATURE  (1<<CPUID_AMD_SVM_PAUSE_FILTER_FEATURE_BIT)
#define CPUID_AMD_SVM_PAUSE_THRESH_FEAT     (1<<CPUID_AMD_SVM_PAUSE_THRESH_FEATURE_BIT)

typedef union amd_svm_features
{
   struct
   {
      uint32_t   eax,ebx,ecx,edx;

   } __attribute__((packed));

   struct
   {
      struct
      {
	 uint32_t    svm_rev:8;    /* SVM revision */
	 uint32_t    rsrvd0:24;

      } __attribute__((packed));

      uint32_t       asid_nr;    /* max number of ASID */
      uint32_t       rsrvd1;

      struct
      {
	 uint32_t    npt:1;           /* Nested Page Tables */
	 uint32_t    lbr:1;           /* LBR virt */
	 uint32_t    lock:1;          /* SVM Lock */
	 uint32_t    nrip:1;          /* Next RIP save */
	 uint32_t    tscrate:1;       /* TSC Rate MSR */
	 uint32_t    vmcb_clean:1;    /* VMCB clean bits */
	 uint32_t    flush_asid:1;    /* Flush by ASID (extended TLB control) */
	 uint32_t    decode_assist:1; /* decode insns */
	 uint32_t    rsrvd3:1;
	 uint32_t    rsrvd4:1;
	 uint32_t    pause:1;         /* pause intercept filter */
	 uint32_t    rsrvd5:1;
         uint32_t    pause_trsh:1;    /* pause intercept filter threshold */
	 uint32_t    rsrvd6:19;

      } __attribute__((packed));

   } __attribute__((packed));

} __attribute__((packed)) amd_svm_feat_t;

#define cpuid_amd_svm_features(feat)					\
   __cpuid_4(CPUID_AMD_SVM_FEATURES,(feat).eax,(feat).ebx,(feat).ecx,(feat).edx)

#define cpuid_check_amd_svm_feature(feature)				\
   ({amd_svm_feat_t feats; cpuid_amd_svm_features(feats);(feats.edx&(feature))?1:0;})

#define svm_lock_supported()					\
   cpuid_check_amd_svm_feature(CPUID_AMD_SVM_LOCK_FEATURE)

#define svm_npt_supported()					\
   cpuid_check_amd_svm_feature(CPUID_AMD_SVM_NPT_FEATURE)

#define svm_lbr_supported()					\
   cpuid_check_amd_svm_feature(CPUID_AMD_SVM_LBR_FEATURE)

#define svm_decode_assist_supported()					\
   cpuid_check_amd_svm_feature(CPUID_AMD_SVM_DECODE_ASSIST_FEATURE)

#define svm_flush_asid_supported()					\
   cpuid_check_amd_svm_feature(CPUID_AMD_SVM_FLUSH_BY_ASID_FEATURE)

#endif
