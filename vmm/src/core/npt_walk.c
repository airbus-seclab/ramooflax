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
** Nested page walking services
**
** . we use VM cpu skillz as we depend
**   upon nested mmu features
*/
int npg_walk(offset_t vaddr, offset_t *paddr)
{
   vm_pgmem_t  *pg = npg_get_active_paging();
   npg_pml4e_t *pml4e;
   npg_pdpe_t  *pdpe;
   npg_pde64_t *pd, *pde;
   npg_pte64_t *pt, *pte;

   __pre_access(npg_cr3);
   debug(PG_W, "nested cr3 0x%X\n", npg_cr3.raw);

   pml4e = &pg->pml4[pml4_idx(vaddr)];
   debug(PG_W, "pml4e @ 0x%X = 0x%X\n", (offset_t)pml4e, pml4e->raw);

   if(!npg_present(pml4e))
   {
      debug(PG_W, "pml4e not present\n");
      return VM_FAIL;
   }

   pdpe = &pg->pdp[pml4_idx(vaddr)][pdp_idx(vaddr)];
   debug(PG_W, "pdpe @ 0x%X = 0x%X\n", (offset_t)pdpe, pdpe->raw);

   if(!npg_present(pdpe))
   {
      debug(PG_W, "pdpe not present\n");
      return VM_FAIL;
   }

   if(info->vm.cpu.skillz.pg_1G && npg_large(pdpe))
   {
      *paddr = pg_1G_addr((offset_t)pdpe->page.addr) + pg_1G_offset(vaddr);
      goto __success;
   }

   pd  = (npg_pde64_t*)page_addr(pdpe->addr);
   pde = &pd[pd64_idx(vaddr)];
   debug(PG_W, "pde @ 0x%X = 0x%X\n", (offset_t)pde, pde->raw);

   if(!npg_present(pde))
   {
      debug(PG_W, "pde not present\n");
      return VM_FAIL;
   }

   if(info->vm.cpu.skillz.pg_2M && npg_large(pde))
   {
      *paddr = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      goto __success;
   }

   pt  = (npg_pte64_t*)page_addr(pde->addr);
   pte = &pt[pt64_idx(vaddr)];
   debug(PG_W, "pte @ 0x%X = 0x%X\n", (offset_t)pte, pte->raw);

   if(!npg_present(pte))
   {
      debug(PG_W, "pte not present\n");
      return VM_FAIL;
   }

   *paddr = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);

__success:
   debug(PG_W, "guest paddr 0x%X -> system paddr 0x%X\n", vaddr, *paddr);
   return VM_DONE;
}
