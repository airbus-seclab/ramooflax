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
#include <emulate_insn.h>
#include <emulate_int.h>
#include <emulate_rmode.h>
#include <vm.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static uint8_t __emulate_get_gpr(struct ud_operand *op)
{
   uint8_t gpr;

   if(op->base < UD_R_RAX)
      gpr = op->base - UD_R_EAX;
   else
      gpr = op->base - UD_R_RAX;

   return (GPR64_RAX - gpr);
}

static int emulate_iret(size_t sz)
{
   offset_t __unused__ addend = 2*sz;

   if(__rmode())
      return emulate_rmode_iret();

   return VM_FAIL;
}

static int emulate_mov(ud_t *disasm)
{
   struct ud_operand  *op1, *op2;
   uint8_t            gpr;

   op1 = &disasm->operand[0];
   op2 = &disasm->operand[1];

   if(op2->type != UD_OP_REG || op1->type != UD_OP_REG)
      return VM_FAIL;

   /* read cr */
   if(range(op2->base, UD_R_CR0, UD_R_CR4))
   {
      uint8_t cr = op2->base - UD_R_CR0;

      if(disasm->pfx_lock != UD_NONE || disasm->pfx_rex != UD_NONE)
	 return VM_FAIL;

      gpr = __emulate_get_gpr(op1);
      return __resolve_cr(0, cr, gpr);
   }
   /* write cr */
   else if(range(op1->base, UD_R_CR0, UD_R_CR4))
   {
      uint8_t cr = op1->base - UD_R_CR0;

      if(disasm->pfx_lock != UD_NONE || disasm->pfx_rex != UD_NONE)
	 return VM_FAIL;

      gpr = __emulate_get_gpr(op2);
      return __resolve_cr(1, cr, gpr);
   }
   /* read dr */
   else if(range(op2->base, UD_R_DR0, UD_R_DR7))
   {
      uint8_t dr = op2->base - UD_R_DR0;
      gpr = __emulate_get_gpr(op1);
      return __resolve_dr(0, dr, gpr);
   }
   /* write dr */
   else if(range(op1->base, UD_R_DR0, UD_R_DR7))
   {
      uint8_t dr = op1->base - UD_R_DR0;
      gpr = __emulate_get_gpr(op2);
      return __resolve_dr(1, dr, gpr);
   }

   return VM_FAIL;
}

static int emulate_sysenter()
{
   seg_sel_t cs;

   __pre_access(__sysenter_cs);
   __pre_access(__sysenter_esp);
   __pre_access(__sysenter_eip);

   cs.raw = __sysenter_cs.wlow;

   if(!__cr0.pe || !cs.index)
   {
      debug(EMU_INSN, "sysenter fault: cr0.pe %d cs.index %d\n", __cr0.pe, cs.index);
      __inject_exception(GP_EXCP, 0, 0);
      return VM_FAULT;
   }

   __rflags.vm = 0;
   __rflags.IF = 0;
   __rflags.rf = 0;

   __cs.selector.raw = __sysenter_cs.wlow;
   __cs.selector.rpl = 0;
   __cs.base.raw = 0ULL;
   __cs.limit.raw = 0xffffffff;
   __cs.attributes.g = 1;
   __cs.attributes.s = 1;
   __cs.attributes.type = SEG_DESC_CODE_XRA;
   __cs.attributes.d = 1;
   __cs.attributes.dpl = 0;
   __cs.attributes.p = 1;
   __cpl = 0;

   __ss.selector.raw = __sysenter_cs.wlow + 8;
   __ss.selector.rpl = 0;
   __ss.base.raw = 0ULL;
   __ss.limit.raw = 0xffffffff;
   __ss.attributes.g = 1;
   __ss.attributes.s = 1;
   __ss.attributes.type = SEG_DESC_DATA_RWA;
   __ss.attributes.d = 1;
   __ss.attributes.dpl = 0;
   __ss.attributes.p = 1;

   __rip.low = __sysenter_eip.low;
   info->vm.cpu.gpr->rsp.low = __sysenter_esp.low;

   __post_access(__rflags);
   __post_access(__rip);

   __post_access(__cs.selector);
   __post_access(__cs.base);
   __post_access(__cs.limit);
   __post_access(__cs.attributes);

   __post_access(__ss.selector);
   __post_access(__ss.base);
   __post_access(__ss.limit);
   __post_access(__ss.attributes);

   return VM_DONE_LET_RIP;
}

static int emulate_sysexit()
{
   seg_sel_t cs;

   __pre_access(__sysenter_cs);

   cs.raw = __sysenter_cs.wlow;

   if(__cpl || !__cr0.pe || !cs.index)
   {
      debug(EMU_INSN, "sysexit fault: cpl %d cr0.pe %d cs.index %d\n"
	    , __cpl, __cr0.pe, cs.index);
      __inject_exception(GP_EXCP, 0, 0);
      return VM_FAULT;
   }

   __cs.selector.raw = __sysenter_cs.wlow + 16;
   __cs.selector.rpl = 3;
   __cs.base.raw = 0ULL;
   __cs.limit.raw = 0xffffffff;
   __cs.attributes.g = 1;
   __cs.attributes.s = 1;
   __cs.attributes.type = SEG_DESC_CODE_XRA;
   __cs.attributes.d = 1;
   __cs.attributes.dpl = 3;
   __cs.attributes.p = 1;
   __cpl = 3;

   __ss.selector.raw = __sysenter_cs.wlow + 24;
   __ss.selector.rpl = 3;
   __ss.base.raw = 0ULL;
   __ss.limit.raw = 0xffffffff;
   __ss.attributes.g = 1;
   __ss.attributes.s = 1;
   __ss.attributes.type = SEG_DESC_DATA_RWA;
   __ss.attributes.d = 1;
   __ss.attributes.dpl = 3;
   __ss.attributes.p = 1;

   info->vm.cpu.gpr->rsp.low = info->vm.cpu.gpr->rcx.low;
   __rip.low = info->vm.cpu.gpr->rdx.low;

   __post_access(__rip);

   __post_access(__cs.selector);
   __post_access(__cs.base);
   __post_access(__cs.limit);
   __post_access(__cs.attributes);

   __post_access(__ss.selector);
   __post_access(__ss.base);
   __post_access(__ss.limit);
   __post_access(__ss.attributes);

   return VM_DONE_LET_RIP;
}

int emulate_clts()
{
   cr0_reg_t guest;

   if(__cpl)
   {
      __inject_exception(GP_EXCP, 0, 0);
      return VM_FAULT;
   }

   guest.low = __cr0.low & ~(CR0_TS);

   __cr0_update(&guest);
   return VM_DONE;
}

int emulate_insn(ud_t *disasm)
{
   size_t mn = disasm->mnemonic;

   switch(mn)
   {
   case UD_Iint:      return emulate_intn(disasm);
   case UD_Imov:      return emulate_mov(disasm);
   case UD_Isysenter: return emulate_sysenter();
   case UD_Isysexit:  return emulate_sysexit();
   case UD_Iiretw:    return emulate_iret(2);
   case UD_Iiretd:    return emulate_iret(4);
   case UD_Iiretq:    return emulate_iret(8);
   case UD_Iclts:     return emulate_clts();
   case UD_Iint1:     return emulate_int1();
   case UD_Iint3:     return emulate_int3();
   case UD_Iinto:     return emulate_into();
   case UD_Ibound:    return emulate_bound();
   }

   return VM_IGNORE;
}
