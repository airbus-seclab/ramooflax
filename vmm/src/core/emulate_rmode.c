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
#include <emulate_rmode.h>
#include <emulate_int15.h>
#include <vm.h>
#include <intr.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void emulate_rmode_push(uint16_t value)
{
   loc_t stack;

   __pre_access(__ss.base);

   info->vm.cpu.gpr->rsp.wlow -= sizeof(uint16_t);
   stack.linear = __ss.base.low + info->vm.cpu.gpr->rsp.wlow;
   *stack.u16   = value;
}

static uint16_t emulate_rmode_pop()
{
   loc_t stack;

   __pre_access(__ss.base);

   stack.linear = __ss.base.low + info->vm.cpu.gpr->rsp.wlow;
   info->vm.cpu.gpr->rsp.wlow += sizeof(uint16_t);
   return *stack.u16;
}

static void emulate_rmode_far_jump(fptr_t *fptr)
{
   __cs.selector.raw = fptr->segment;
   __cs.base.low = (__cs.selector.raw)<<4;

   /* on op16, eip high is cleared */
   __rip.low = fptr->offset.raw;

   debug(EMU_RMODE_FAR, "far jump to 0x%x:0x%x\n",
   	 __cs.selector.raw, __rip.wlow);

   __post_access(__cs.selector);
   __post_access(__cs.base);
   __post_access(__rip);
}

static void emulate_rmode_far_call(fptr_t *fptr, uint16_t insn_sz)
{
   emulate_rmode_push(__cs.selector.raw);
   emulate_rmode_push(__rip.wlow+insn_sz);

   debug(EMU_RMODE_FAR,
	 "far call saved frame 0x%x:0x%x\n",
   	 __cs.selector.raw, __rip.wlow+insn_sz);

   emulate_rmode_far_jump(fptr);
}

static void emulate_rmode_far_ret(uint16_t add_sp)
{
   __rip.wlow        = emulate_rmode_pop();
   __cs.selector.raw = emulate_rmode_pop();
   __cs.base.low     = (__cs.selector.raw)<<4;

   debug(EMU_RMODE_FAR, "far ret to 0x%x:0x%x\n", __cs.selector.raw, __rip.wlow);

   if(add_sp)
      info->vm.cpu.gpr->rsp.wlow += add_sp;

   __post_access(__rip);
   __post_access(__cs.base);
   __post_access(__cs.selector);
}

int emulate_rmode_iret()
{
   emulate_rmode_far_ret(0);
   __rflags.wlow = emulate_rmode_pop();
   __post_access(__rflags);

   debug(EMU_RMODE_IRET, "iret\n");
   return VM_DONE_LET_RIP;
}

int emulate_rmode_interrupt(uint8_t vector, uint16_t insn_sz)
{
   ivt_e_t *ivt;
   fptr_t  fptr;

   debug(EMU_RMODE_INT, "int 0x%x (ax 0x%x)\n"
	 , vector, info->vm.cpu.gpr->rax.wlow);

   if(vector == BIOS_MISC_INTERRUPT)
   {
      int rc = emulate_int15();

      if(rc != VM_DONE_LET_RIP)
	 return rc;
   }

   ivt = (ivt_e_t*)0;

   fptr.segment      = ivt[vector].cs;
   fptr.offset.wlow  = ivt[vector].ip;
   fptr.offset.whigh = 0;

   emulate_rmode_push(__rflags.wlow);

   __rflags.IF = 0;
   __rflags.tf = 0;
   __rflags.rf = 0;
   __rflags.ac = 0;

   emulate_rmode_far_call(&fptr, insn_sz);
   __post_access(__rflags);

   return VM_DONE_LET_RIP;
}
