/*
** Copyright (C) 2016 Airbus Group, stephane duverger <stephane.duverger@airbus.com>
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
#include <npg.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** Nested page walking service
**
** . we use VM cpu skillz as we depend
**   upon nested mmu features
*/
int __npg_walk(vm_pgmem_t *pg, offset_t vaddr, pg_wlk_t *wlk)
{
   npg_pml4e_t *pml4e;
   npg_pdpe_t  *pdp,*pdpe;
   npg_pde64_t *pd, *pde;
   npg_pte64_t *pt, *pte;

   debug(PG_WLK, "(n)cr3 0x%X\n", pg->pml4);

   wlk->attr = 0;
   wlk->u = 1;

   pml4e = &pg->pml4[pml4_idx(vaddr)];
   debug(PG_WLK, "(n)pml4e @ 0x%X = 0x%X\n", (offset_t)pml4e, pml4e->raw);

   if(!npg_present(pml4e))
   {
      debug(PG_WLK, "(n)pml4e not present\n");
      wlk->type  = PG_WALK_TYPE_PML4E;
      wlk->entry = (void*)pml4e;
      return VM_FAULT;
   }

   wlk->r = npg_readable(pml4e);
   wlk->w = npg_writable(pml4e);
   wlk->x = npg_executable(pml4e);

   pdp  = (npg_pdpe_t*)page_addr(pml4e->addr);
   pdpe = &pdp[pdp_idx(vaddr)];
   debug(PG_WLK, "(n)pdpe @ 0x%X = 0x%X\n", (offset_t)pdpe, pdpe->raw);

   if(!npg_present(pdpe))
   {
      debug(PG_WLK, "(n)pdpe not present\n");
      wlk->type  = PG_WALK_TYPE_PDPE;
      wlk->entry = (void*)pdpe;
      return VM_FAULT;
   }

   wlk->r &= npg_readable(pdpe);
   wlk->w &= npg_writable(pdpe);
   wlk->x &= npg_executable(pdpe);

   if(info->vm.cpu.skillz.pg_1G && npg_large(pdpe))
   {
      wlk->addr  = pg_1G_addr((offset_t)pdpe->page.addr) + pg_1G_offset(vaddr);
      wlk->type  = PG_WALK_TYPE_PDPE;
      wlk->size  = PG_1G_SIZE;
      wlk->entry = (void*)pdpe;
      goto __success;
   }

   pd  = (npg_pde64_t*)page_addr(pdpe->addr);
   pde = &pd[pd64_idx(vaddr)];
   debug(PG_WLK, "(n)pde @ 0x%X = 0x%X\n", (offset_t)pde, pde->raw);

   if(!npg_present(pde))
   {
      debug(PG_WLK, "(n)pde not present\n");
      wlk->type  = PG_WALK_TYPE_PDE64;
      wlk->entry = (void*)pde;
      return VM_FAULT;
   }

   wlk->r &= npg_readable(pde);
   wlk->w &= npg_writable(pde);
   wlk->x &= npg_executable(pde);

   if(info->vm.cpu.skillz.pg_2M && npg_large(pde))
   {
      wlk->addr  = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      wlk->type  = PG_WALK_TYPE_PDE64;
      wlk->size  = PG_2M_SIZE;
      wlk->entry = (void*)pde;
      goto __success;
   }

   pt  = (npg_pte64_t*)page_addr(pde->addr);
   pte = &pt[pt64_idx(vaddr)];
   debug(PG_WLK, "(n)pte @ 0x%X = 0x%X\n", (offset_t)pte, pte->raw);

   if(!npg_present(pte))
   {
      debug(PG_WLK, "(n)pte not present\n");
      wlk->type  = PG_WALK_TYPE_PTE64;
      wlk->entry = (void*)pte;
      return VM_FAULT;
   }

   wlk->addr  = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);
   wlk->type  = PG_WALK_TYPE_PTE64;
   wlk->size  = PG_4K_SIZE;
   wlk->entry = (void*)pte;
   wlk->r    &= npg_readable(pte);
   wlk->w    &= npg_writable(pte);
   wlk->x    &= npg_executable(pte);

__success:
   debug(PG_WLK, "guest paddr 0x%X -> system paddr 0x%X\n", vaddr, wlk->addr);
   return VM_DONE;
}
