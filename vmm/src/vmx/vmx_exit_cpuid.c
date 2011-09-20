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
#include <vmx_exit_cpuid.h>
#include <vmx_cpuid.h>
#include <info_data.h>

extern info_data_t *info;

void __vmx_vmexit_resolve_cpuid(uint32_t idx)
{
   switch(idx)
   {
   case CPUID_FEATURE_INFO:
      __vmx_vmexit_resolve_cpuid_feature();
      break;
   default:
      break;
   }
}

void __vmx_vmexit_resolve_cpuid_feature()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   ctx->rcx.low &= ~CPUID_ECX_FEAT_VMX;
}
