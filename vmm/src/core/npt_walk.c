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
#include <paging.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** VM nested page walking services using VM cpu skillz
** as we depend upon NPT features
*/
int npg_walk(offset_t vaddr, offset_t *paddr)
{
   npg_pml4e_t *pml4e;
   npg_pdpe_t  *pdpe;
   npg_pde64_t *pd, *pde;
   npg_pte64_t *pt, *pte;

   debug(PG_W, "npg_walk on 0x%X\n", vaddr);
   debug(PG_W, "nested CR3: 0x%X\n", npg_cr3.raw);

   pml4e = &info->vm.cpu.pg.pml4[pml4_idx(vaddr)];

   debug(PG_W, "pml4e @ 0x%X = %x %x\n", (offset_t)pml4e, pml4e->high, pml4e->low);
   if(!npg_present(pml4e))
   {
      debug(PG_W, "pml4e not present\n");
      return 0;
   }

   pdpe = &info->vm.cpu.pg.pdp[pml4_idx(vaddr)][pdp_idx(vaddr)];

   debug(PG_W, "pdpe @ 0x%X = %x %x\n", (offset_t)pdpe, pdpe->high, pdpe->low);
   if(!npg_present(pdpe))
   {
      debug(PG_W, "pdpe not present\n");
      return 0;
   }

   if(info->vm.cpu.skillz.pg_1G && npg_large(pdpe))
   {
      *paddr = pg_1G_addr((offset_t)pdpe->page.addr) + pg_1G_offset(vaddr);
      goto __success;
   }

   pd  = (npg_pde64_t*)page_addr(pdpe->addr);
   pde = &pd[pd64_idx(vaddr)];

   debug(PG_W, "pde @ 0x%X = %x %x\n", (offset_t)pde, pde->high, pde->low);
   if(!npg_present(pde))
   {
      debug(PG_W, "pde not present\n");
      return 0;
   }

   if(info->vm.cpu.skillz.pg_2M && npg_large(pde))
   {
      *paddr = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      goto __success;
   }

   pt  = (npg_pte64_t*)page_addr(pde->addr);
   pte = &pt[pt64_idx(vaddr)];

   debug(PG_W, "pte @ 0x%X = %x %x\n", (offset_t)pte, pte->high, pte->low);
   if(!npg_present(pte))
   {
      debug(PG_W, "pte not present\n");
      return 0;
   }

   *paddr = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);

__success:
   debug(PG_W, "paddr 0x%X -> nested addr 0x%X\n", vaddr, *paddr);
   return 1;
}
