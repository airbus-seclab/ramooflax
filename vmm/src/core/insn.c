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
#include <insn.h>
#include <vmm.h>
#include <emulate.h>
#include <emulate_int.h>
#include <intr.h>
#include <ctrl.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

int resolve_hypercall()
{
   ctrl_evt_hypercall();
   return emulate_done(VM_DONE, max(__insn_sz(), HYPERCALL_INSN_SZ));
}

int resolve_invd()
{
   emulate_native();
   invd();
   return emulate_done(VM_DONE, max(__insn_sz(), INVD_INSN_SZ));
}

int resolve_wbinvd()
{
   emulate_native();
   wbinvd();
   return emulate_done(VM_DONE, max(__insn_sz(), WBINVD_INSN_SZ));
}

int resolve_hlt()
{
   emulate_native();
   return VM_FAIL;
}

int resolve_icebp()
{
   return emulate_done(emulate_int1(), max(__insn_sz(), 1));
}

int resolve_xsetbv()
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;
   xcr0_reg_t   xcr0;

   emulate_native();

   xcr0.low  = ctx->rax.low;
   xcr0.high = ctx->rdx.low;

   if(ctx->rcx.low || !xcr0.x87 || (xcr0.avx && !xcr0.sse))
   {
      __inject_exception(GP_EXCP, 0, 0);
      return VM_FAULT;
   }

   xsetbv(ctx->rax.low, ctx->rcx.low, ctx->rdx.low);
   return emulate_done(VM_DONE, max(__insn_sz(), XSETBV_INSN_SZ));
}

int resolve_default()
{
   return VM_FAIL;
}
