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
#include <svm_exit_cpuid.h>
#include <svm_cpuid.h>
#include <info_data.h>

extern info_data_t *info;

static void __svm_vmexit_resolve_cpuid_phys_core()
{
   /* gpr64_ctx_t         *ctx = info->vm.cpu.gpr; */
   /* amd_phys_core_ecx_t *ecx = (amd_phys_core_ecx_t*)&ctx->rcx.low; */

   /* ecx->nc = 0; */
   /* ecx->apic_id_core_id_sz = 0; */
}

static void __svm_vmexit_resolve_cpuid_feature()
{
   /* gpr64_ctx_t           *ctx = info->vm.cpu.gpr; */
   /* cpuid_feat_info_ebx_t *ebx = (cpuid_feat_info_ebx_t*)&ctx->rbx.low; */

   /* ebx->logical_cpu_nr = 1; */
}

static void __svm_vmexit_resolve_cpuid_ext_proc_feat()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   ctx->rcx.low &= ~(CPUID_AMD_ECX_EXT_PROC_FEAT_SVM|CPUID_AMD_ECX_EXT_PROC_FEAT_CMP);
}

void __svm_vmexit_resolve_cpuid(uint32_t idx)
{
   switch(idx)
   {
   case CPUID_EXT_PROC_FEAT:
      __svm_vmexit_resolve_cpuid_ext_proc_feat();
      break;
   case CPUID_FEATURE_INFO:
      __svm_vmexit_resolve_cpuid_feature();
      break;
   case CPUID_AMD_PHYS_CORE:
      __svm_vmexit_resolve_cpuid_phys_core();
      break;
   default:
      break;
   }
}

