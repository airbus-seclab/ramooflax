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
#include <dr.h>
#include <vmm.h>
#include <vm.h>
#include <info_data.h>
#include <excp.h>
#include <debug.h>

extern info_data_t *info;

offset_t get_dr(uint8_t n)
{
   if(n>3) n &= 3;

   switch(n)
   {
   case 0: return __get_dr(0);
   case 1: return __get_dr(1);
   case 2: return __get_dr(2);
   }

   return __get_dr(3);
}

void set_dr(uint8_t n, offset_t addr)
{
   if(n>3) n &= 3;

   switch(n)
   {
   case 0: __set_dr(0, addr); return;
   case 1: __set_dr(1, addr); return;
   case 2: __set_dr(2, addr); return;
   }

   __set_dr(3, addr);
}

/*
** XXX: __lmode64() must check rex.w and rex.r for 64 bits op and r8/r15 access
*/
static int __resolve_dr_rd(uint8_t dr, uint8_t gpr)
{
   info->vm.cpu.gpr->raw[gpr].low = info->vm.dr_shadow[dr].low;
   return DR_SUCCESS;
}

/*
** XXX: __lmode64() must check rex.w and rex.r for 64 bits op and r8/r15 access
*/
static int __resolve_dr_wr(uint8_t dr, uint8_t gpr)
{
   debug(DR, "write 0%x to dr%d\n", info->vm.cpu.gpr->raw[gpr].low, dr);

   info->vm.dr_shadow[dr].low = info->vm.cpu.gpr->raw[gpr].low;

   /* reserved bits */
   if(dr == 4)
   {
      info->vm.dr_shadow[dr].low &= 0xfffffeff;
      info->vm.dr_shadow[dr].low |= 0xffff0ff0;
   }
   else if(dr == 5)
   {
      info->vm.dr_shadow[dr].low &= 0xffff23ff;
      info->vm.dr_shadow[dr].low |= 0x400;
   }

   return DR_SUCCESS;
}

int __resolve_dr(uint8_t wr, uint8_t dr, uint8_t gpr)
{
   if(!__valid_dr_access())
   {
      __inject_exception(GP_EXCP, 0, 0);
      return DR_FAULT;
   }

   if(!__valid_dr_regs(gpr, dr))
      return DR_FAIL;

   if(__cr4.de && (dr == 4 || dr == 5))
   {
      __inject_exception(UD_EXCP, 0, 0);
      return DR_FAULT;
   }

   if(dr > 5)
      dr -= 2;

   /* shadow dr7 general detect */
   if(dr == 5 && (info->vm.dr_shadow[dr].low & DR7_GD))
   {
      info->vm.dr_shadow[4].low |= DR6_BD;
      info->vm.dr_shadow[5].low &= ~DR7_GD;
      __inject_exception(DB_EXCP, 0, 0);
      return DR_FAULT;
   }

   if(wr)
      return __resolve_dr_wr(dr, gpr);

   return __resolve_dr_rd(dr, gpr);
}
