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
#include <cpuid.h>
#include <vmm.h>
#include <emulate.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

static void __resolve_cpuid_native()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   emulate_native();
   __cpuid(ctx->rax.low, ctx->rbx.low, ctx->rcx.low, ctx->rdx.low);
}

static void __resolve_cpuid_feature()
{
   /* gpr64_ctx_t *ctx = info->vm.cpu.gpr; */
   /* ctx->rcx.low &= ~CPUID_ECX_FEAT_MONITOR; */
   /* ctx->rdx.low &= ~CPUID_EDX_FEAT_APIC; */
}

static int __resolve_cpuid()
{
   uint32_t idx = info->vm.cpu.gpr->rax.low;

   __resolve_cpuid_native();
   __resolve_cpuid_arch(idx);

   switch(idx)
   {
   case CPUID_FEATURE_INFO:
      __resolve_cpuid_feature();
      break;
   default:
      break;
   }

   debug(CPUID, "cpuid 0x%x | 0x%x 0x%x 0x%x 0x%x\n"
	 ,idx
	 ,info->vm.cpu.gpr->rax.low
	 ,info->vm.cpu.gpr->rbx.low
	 ,info->vm.cpu.gpr->rcx.low
	 ,info->vm.cpu.gpr->rdx.low
      );

   return VM_DONE;
}

int resolve_cpuid()
{
   return emulate_done(__resolve_cpuid(), max(__insn_sz(), CPUID_INSN_SZ));
}

