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
#include <ctrl.h>
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

static void __resolve_cpuid_mshyperv_zero()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;
   ctx->rax.low = ctx->rbx.low = ctx->rcx.low = ctx->rdx.low = 0;
}

static int __resolve_cpuid_mshyperv(uint32_t idx)
{
   switch(idx)
   {
   case CPUID_MSHYPERV_RANGE:
   case CPUID_MSHYPERV_ID:
   case CPUID_MSHYPERV_FEAT:
      __resolve_cpuid_mshyperv_zero();
      return VM_DONE;
   }

   return VM_IGNORE;
}

#ifndef CONFIG_CPUID_QEMU_NONE
static int __resolve_cpuid_qemu_basic_info()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   ctx->rax.low = 0x00000004;
   ctx->rbx.low = 0x68747541; /* [Auth] */
   ctx->rcx.low = 0x444d4163; /* [cAMD] */
   ctx->rdx.low = 0x69746e65; /* [enti] */

   return VM_DONE;
}

static int __resolve_cpuid_qemu_feat_info()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   ctx->rax.low = 0x00000663;
   ctx->rbx.low = 0x00000800;
   ctx->rcx.low = 0x80802001;
   ctx->rdx.low = 0x078bfbf9;

   return VM_DONE;
}

static int __resolve_cpuid_qemu_max_ext()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   ctx->rax.low = 0x8000000a;
   ctx->rbx.low = 0x68747541; /* [Auth] */
   ctx->rcx.low = 0x444d4163; /* [cAMD] */
   ctx->rdx.low = 0x69746e65; /* [enti] */

   return VM_DONE;
}

static int __resolve_cpuid_qemu_proc_serial()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   ctx->rax.low = 0;
   ctx->rbx.low = 0;
   ctx->rcx.low = 0;
   ctx->rdx.low = 0;

   return VM_DONE;
}

static int __resolve_cpuid_qemu_brand_str1()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   ctx->rax.low = 0x554d4551; /* [QEMU] */
   ctx->rbx.low = 0x72695620; /* [ Vir] */
   ctx->rcx.low = 0x6c617574; /* [tual] */
   ctx->rdx.low = 0x55504320; /* [ CPU] */

   return VM_DONE;
}

static int __resolve_cpuid_qemu_brand_str2()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   ctx->rax.low = 0x72657620; /* [ ver] */
   ctx->rbx.low = 0x6e6f6973; /* [sion] */
   ctx->rcx.low = 0x302e3220; /* [ 2.0] */
   ctx->rdx.low = 0x0000302e; /* [.0]   */

   return VM_DONE;
}

static int __resolve_cpuid_qemu_brand_str3()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   ctx->rax.low = 0;
   ctx->rbx.low = 0;
   ctx->rcx.low = 0;
   ctx->rdx.low = 0;

   return VM_DONE;
}

static int __resolve_cpuid_qemu(uint32_t idx)
{
#ifdef CONFIG_CPUID_QEMU_USER
   if(__cpl == 3)
#endif
      switch(idx)
      {
      case CPUID_MAX_EXT:      return __resolve_cpuid_qemu_max_ext();
      case CPUID_BASIC_INFO:   return __resolve_cpuid_qemu_basic_info();
      case CPUID_FEATURE_INFO: return __resolve_cpuid_qemu_feat_info();
      case CPUID_PROC_SERIAL:  return __resolve_cpuid_qemu_proc_serial();
      case CPUID_BRAND_STR1:   return __resolve_cpuid_qemu_brand_str1();
      case CPUID_BRAND_STR2:   return __resolve_cpuid_qemu_brand_str2();
      case CPUID_BRAND_STR3:   return __resolve_cpuid_qemu_brand_str3();
      }

   return VM_IGNORE;
}
#endif

static int __resolve_cpuid()
{
   uint32_t idx = info->vm.cpu.gpr->rax.low;

#ifndef CONFIG_CPUID_QEMU_NONE
   if(__resolve_cpuid_qemu(idx) == VM_DONE)
      goto __leave;
#endif

   if(__resolve_cpuid_mshyperv(idx) == VM_DONE)
      goto __leave;

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

__leave:
   debug(CPUID, "cpuid 0x%x | 0x%x 0x%x 0x%x 0x%x\n"
	 ,idx
	 ,info->vm.cpu.gpr->rax.low
	 ,info->vm.cpu.gpr->rbx.low
	 ,info->vm.cpu.gpr->rcx.low
	 ,info->vm.cpu.gpr->rdx.low
      );

   ctrl_evt_cpuid(idx);
   return VM_DONE;
}

int resolve_cpuid()
{
   return emulate_done(__resolve_cpuid(), max(__insn_sz(), CPUID_INSN_SZ));
}

