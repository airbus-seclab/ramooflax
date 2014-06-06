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
#include <emulate_int.h>
#include <emulate_rmode.h>
#include <disasm.h>
#include <vm.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int emulate_interrupt(uint8_t vector, uint16_t insn_sz)
{
   if(__rmode())
      return emulate_rmode_interrupt(vector, insn_sz);

   return VM_FAIL;
}

int emulate_hard_interrupt(uint8_t vector)
{
   return emulate_interrupt(vector, 0);
}

int emulate_exception(uint8_t vector)
{
   return emulate_interrupt(vector, 0);
}

int emulate_int1()
{
   /* icebp */
   return VM_FAIL;
}

int emulate_int3()
{
   return VM_FAIL;
}

int emulate_into()
{
   if(!__rflags.of)
      return VM_DONE;

   return VM_FAIL;
}

int emulate_bound()
{
   return VM_FAIL;
}

int emulate_intn(ud_t *disasm)
{
   struct ud_operand *op = &disasm->operand[0];

   if(op->type != UD_OP_IMM)
   {
      debug(EMU_INSN, "intN bad operand\n");
      return VM_FAIL;
   }

   return emulate_interrupt(op->lval.ubyte, ud_insn_len(disasm));
}
