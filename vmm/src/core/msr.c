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
#include <msr.h>
#include <vmm.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

#ifdef __MSR_PROXY__
static int __resolve_msr_mtrr_def_type(uint8_t wr)
{
   gpr64_ctx_t          *ctx = info->vm.cpu.gpr;
   ia32_mtrr_def_type_t def;

   def.eax = ctx->rax.low;
   def.edx = ctx->rdx.low;

   if(wr)
   {
      debug(MSR, "%sabling mtrr\n", def.e?"en":"dis");
      return MSR_NATIVE;
   }
   else
      return MSR_SUCCESS;
}

static int __resolve_msr_common(uint8_t wr)
{
   switch(info->vm.cpu.gpr->rcx.low)
   {
   case IA32_MTRR_DEF_TYPE:
      return __resolve_msr_mtrr_def_type(wr);
   default:
      return MSR_FAIL;
   }
}

static int __resolve_msr_rd()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   __rd_msr(ctx->rax.low, ctx->rcx.low, ctx->rdx.low);

   debug(MSR, "rdmsr 0x%x | 0x%x 0x%x\n", ctx->rcx.low, ctx->rdx.low, ctx->rax.low);

   if(__resolve_msr_arch(0) != MSR_SUCCESS)
      __resolve_msr_common(0);

   return MSR_SUCCESS;
}

static int __resolve_msr_wr()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   debug(MSR, "wrmsr 0x%x | 0x%x 0x%x\n", ctx->rcx.low, ctx->rdx.low, ctx->rax.low);

   if(__resolve_msr_arch(1) != MSR_SUCCESS)
   {
      __resolve_msr_common(1);
      __wr_msr(ctx->rax.low, ctx->rcx.low, ctx->rdx.low);
   }

   return MSR_SUCCESS;
}
#else
static int __resolve_msr_common()
{
   switch(info->vm.cpu.gpr->rcx.low)
   {
   default:
      return MSR_FAIL;
   }
}

static int __resolve_msr_rd()
{
   int         rc;
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   __rd_msr(ctx->rax.low, ctx->rcx.low, ctx->rdx.low);

   rc = __resolve_msr_arch(0);

   if(rc == MSR_FAIL)
      rc = __resolve_msr_common(0);

   return rc;
}

static int __resolve_msr_wr()
{
   int rc = __resolve_msr_arch(1);

   if(rc == MSR_FAIL)
      rc = __resolve_msr_common();

    if(rc == MSR_NATIVE)
    {
      gpr64_ctx_t *ctx = info->vm.cpu.gpr;
      __wr_msr(ctx->rax.low, ctx->rcx.low, ctx->rdx.low);
       rc = MSR_SUCCESS;
    }

    return rc;
}
#endif

int resolve_msr(uint8_t wr)
{
   int rc;

   if(wr)
      rc = __resolve_msr_wr();
   else
      rc  =__resolve_msr_rd();

   if(rc == MSR_FAIL)
      return 0;

   if(rc == MSR_SUCCESS)
      vm_update_rip(MSR_INSN_SZ);
   else
      debug(MSR, "msr strange 0x%x\n", info->vm.cpu.gpr->rcx.low);

   return 1;
}
