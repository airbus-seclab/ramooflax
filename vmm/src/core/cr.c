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
#include <cr.h>
#include <vmm.h>
#include <vm.h>
#include <ctrl.h>
#include <info_data.h>
#include <excp.h>
#include <debug.h>

extern info_data_t *info;

int __resolve_cr0_wr(cr0_reg_t *guest)
{
   uint32_t cr0_update, updated=0;

   __pre_access(__cr4);

   cr0_update = __cr0.low ^ guest->low;

   debug(CR0, "wr cr0 0x%x\n", guest->low);

   if(__invalid_cr0_setup(guest) || __invalid_cr0_lmode_check(guest, cr0_update))
   {
      debug(CR0, "invalid cr0 setup\n");
      __inject_exception(GP_EXCP, 0, 0);
      return CR_FAULT;
   }

   if(cr0_update & (CR0_NW|CR0_CD))
   {
      __cr0_cache_update(guest);
      updated = 1;
   }

   if(cr0_update & (CR0_PE|CR0_PG))
   {
      __flush_tlb_glb();

      if(cr0_update & CR0_PG)
      {
	 updated = 1;
	 __efer_update(guest->pg);
      }
   }

   if(cr0_update & CR0_PE)
   {
      if(!guest->pe)
	 vm_enter_rmode();
      else
	 vm_enter_pmode();
   }

   __cr0_update(guest);

   if(!__efer.lma && updated && __cr0.pg && __cr4.pae && !__update_npg_pdpe())
   {
      debug(CR0, "pae pdpe update fault\n");
      __inject_exception(GP_EXCP, 0, 0);
      return CR_FAULT;
   }

   return CR_SUCCESS;
}

int __resolve_cr3_wr(cr3_reg_t *guest)
{
   __pre_access(__cr4);

   if(__lmode64())
      __cr3.raw = guest->raw;
   else
      __cr3.low = guest->low;

   debug(CR3, "wr cr3 0x%X\n", guest->raw);

   __update_npg_cache(guest);
   __flush_tlb();

   if(!__efer.lma && __cr0.pg && __cr4.pae && !__update_npg_pdpe())
   {
      debug(CR3, "pae pdpe update fault\n");
      __inject_exception(GP_EXCP, 0, 0);
      return CR_FAULT;
   }

   __post_access(__cr3);
   return CR_SUCCESS;
}

int __resolve_cr4_wr(cr4_reg_t *guest)
{
   uint32_t cr4_update, updated=0;

   __pre_access(__cr4);

   cr4_update = __cr4.low ^ guest->low;

   debug(CR4, "wr cr4 0x%x\n", guest->low);

   if(cr4_update & (CR4_PAE|CR4_PSE|CR4_PGE))
   {
      __flush_tlb_glb();
      updated = 1;
   }

#ifndef CONFIG_ARCH_AMD
   if(__efer.lma && (cr4_update & CR4_PAE) && !guest->pae)
   {
      debug(CR4, "disable pae while in lmode #GP\n");
      __inject_exception(GP_EXCP, 0, 0);
      return CR_FAULT;
   }
#endif

   __cr4_update(guest);

   if(!__efer.lma && updated && __cr0.pg && __cr4.pae && !__update_npg_pdpe())
   {
      debug(CR4, "pae pdpe update fault\n");
      __inject_exception(GP_EXCP, 0, 0);
      return CR_FAULT;
   }

   return CR_SUCCESS;
}

int __resolve_cr_wr_with(uint8_t cr, raw64_t *val)
{
   int rc;

   if(cr == 0)
      rc = __resolve_cr0_wr((cr0_reg_t*)val);
   else if(cr == 3)
      rc = __resolve_cr3_wr((cr3_reg_t*)val);
   else if(cr == 4)
      rc = __resolve_cr4_wr((cr4_reg_t*)val);
   else
      return CR_FAIL;

   ctrl_evt_cr_wr(cr);
   return rc;
}

static int __resolve_cr_wr(uint8_t cr, uint8_t gpr)
{
   uint64_t creg = info->vm.cpu.gpr->raw[gpr].raw;

   return __resolve_cr_wr_with(cr, (raw64_t*)&creg);
}

static int __resolve_cr_rd(uint8_t cr, uint8_t gpr)
{
   raw64_t *creg;

   if(cr == 0)
      creg = (raw64_t*)&__cr0.raw;
   else if(cr == 3)
   {
      creg = (raw64_t*)&__cr3.raw;
   }
   else if(cr == 4)
   {
      __pre_access(__cr4);
      creg = (raw64_t*)&__cr4.raw;
   }
   else
      return CR_FAIL;

   /* XXX: check rex.r for r8/r15 access */
   if(__lmode64())
      info->vm.cpu.gpr->raw[gpr].raw = creg->raw;
   else
      info->vm.cpu.gpr->raw[gpr].low = creg->low;

   ctrl_evt_cr_rd(cr);
   return CR_SUCCESS;
}

int __resolve_cr(uint8_t wr, uint8_t cr, uint8_t gpr)
{
   if(!__valid_cr_access())
   {
      __inject_exception(GP_EXCP, 0, 0);
      return CR_FAULT;
   }

   if(!__valid_cr_regs(gpr, cr))
      return CR_FAIL;

   if(wr)
      return __resolve_cr_wr(cr, gpr);

   return __resolve_cr_rd(cr, gpr);
}
