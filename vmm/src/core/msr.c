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
#include <emulate.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int __resolve_msr_rd()
{
   int rc = __resolve_msr_arch(0);

   if(rc == VM_NATIVE)
   {
      gpr64_ctx_t *ctx = info->vm.cpu.gpr;
      emulate_native();
      __rd_msr(ctx->rax.low, ctx->rcx.low, ctx->rdx.low);
      rc = VM_DONE;
   }

   return rc;
}

static int __resolve_msr_wr()
{
   /* XXX: check reserved bits and raise #GP */
   int rc = __resolve_msr_arch(1);

   if(rc == VM_NATIVE)
   {
      gpr64_ctx_t *ctx = info->vm.cpu.gpr;
      emulate_native();
      __wr_msr(ctx->rax.low, ctx->rcx.low, ctx->rdx.low);
      rc = VM_DONE;
   }

   return rc;
}

int resolve_msr(uint8_t wr)
{
   /* XXX: check reserved/unimplemented MSR and raise #GP */
   int rc = wr ? __resolve_msr_wr() : __resolve_msr_rd();

   debug(MSR, "%smsr 0x%x | 0x%x 0x%x\n"
	 , wr?"wr":"rd", info->vm.cpu.gpr->rcx.low
	 , info->vm.cpu.gpr->rdx.low, info->vm.cpu.gpr->rax.low);

   return emulate_done(rc, max(__insn_sz(), MSR_INSN_SZ));
}
