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
#include <gdb.h>
#include <info_data.h>
#include <excp.h>
#include <debug.h>

extern info_data_t *info;

int __resolve_cr0_wr(cr0_reg_t *guest)
{
   uint32_t cr0_update = __cr0.low ^ guest->low;

   if(__invalid_cr0_setup(guest) || __invalid_cr0_lmode_check(guest, cr0_update))
   {
      __inject_exception(GP_EXCP, 0, 0);
      return CR_FAULT;
   }

   if(cr0_update & CR0_CD)
   {
      cr0_reg_t cr0;
      cr0.raw = get_cr0();
      cr0.cd = guest->cd;

      debug(CR, "cr0 cache disable = %d\n", cr0.cd);
      set_cr0(cr0.raw);
   }

   if(cr0_update & (CR0_PE|CR0_PG))
      __flush_tlb_glb();

   if(cr0_update & CR0_PE)
   {
      if(!guest->pe)
	 vm_enter_rmode();
      else
	 vm_enter_pmode();
   }

   __cr0.low = guest->low;
   __post_access_restrictive(__cr0, 0);

   return CR_SUCCESS;
}

int __resolve_cr3_wr(cr3_reg_t *guest)
{
   if(__lmode64())
      __cr3.raw = guest->raw;
   else
      __cr3.low = guest->low;

   debug(CR, "wr cr3 0x%X\n", guest->raw);

   __ncr3.pml4.pwt = guest->pwt;
   __ncr3.pml4.pcd = guest->pcd;

   __flush_tlb();
   __post_access(__cr3);

   if(info->vmm.ctrl.dbg.status.cr3 && __cr3.low == info->vmm.ctrl.dbg.stored_cr3.low)
      debug(CR, "kernel reloaded active cr3\n");

   return CR_SUCCESS;
}

int __resolve_cr4_wr(cr4_reg_t *guest)
{
   uint32_t cr4_update = __cr4.low ^ guest->low;

   debug(CR, "wr cr4 0x%X\n", guest->raw);

   if(cr4_update & (CR4_PAE|CR4_PSE|CR4_PGE))
      __flush_tlb_glb();

   __cr4.low = guest->low;
   __post_access_restrictive(__cr4, 4);

   return CR_SUCCESS;
}

static int __resolve_cr_wr(uint8_t cr, uint8_t gpr)
{
   int      rc;
   uint64_t creg = info->vm.cpu.gpr->raw[gpr].raw;

   gdb_cr_wr_event(cr);

   if(cr == 0)
      rc = __resolve_cr0_wr((cr0_reg_t*)&creg);
   else if(cr == 3)
      rc = __resolve_cr3_wr((cr3_reg_t*)&creg);
   else if(cr == 4)
      rc = __resolve_cr4_wr((cr4_reg_t*)&creg);
   else
      return CR_FAIL;

   return rc;
}

static int __resolve_cr_rd(uint8_t cr, uint8_t gpr)
{
   raw64_t *creg;

   if(cr == 0)
      creg = (raw64_t*)&__cr0.raw;
   else if(cr == 3)
      creg = (raw64_t*)&__cr3.raw;
   else if(cr == 4)
      creg = (raw64_t*)&__cr4.raw;
   else
      return CR_FAIL;

   /* XXX: check rex.r for r8/r15 access */
   if(__lmode64())
      info->vm.cpu.gpr->raw[gpr].raw = creg->raw;
   else
      info->vm.cpu.gpr->raw[gpr].low = creg->low;

   gdb_cr_rd_event(cr);
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

/* int resolve_cr(uint8_t wr, uint8_t src, uint8_t dst) */
/* { */
/*    int rc = __resolve_cr(wr, src, dst); */

/*    if(rc == CR_FAIL) */
/*       return 0; */

/*    if(rc == CR_SUCCESS) */
/*       vm_update_rip(MOV_CR_INSN_SZ); */

/*    return 1; */
/* } */
