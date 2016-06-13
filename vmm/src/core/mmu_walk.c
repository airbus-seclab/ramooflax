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
#include <paging.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** lmode: 1GB, 2MB and 4KB pages
** cr4.pse is ignored
** 1GB cpuid feature must be checked
*/
static int __pg_walk_lmode(cr3_reg_t *cr3, offset_t vaddr, pg_wlk_t *wlk)
{
   pml4e_t *pml4, *pml4e;
   pdpe_t  *pdp, *pdpe;
   pde64_t *pd, *pde;
   pte64_t *pt, *pte;

   pml4 = (pml4e_t*)page_addr(cr3->pml4.addr);
   if(vmm_area_range(pml4, PG_4K_SIZE))
   {
      debug(PG_WLK, "pml4 in vmm area\n");
      return VM_FAIL;
   }

   pml4e = &pml4[pml4_idx(vaddr)];
   debug(PG_WLK, "pml4e @ 0x%X = %X\n", (offset_t)pml4e, pml4e->raw);

   if(!pg_present(pml4e))
   {
      debug(PG_WLK, "pml4e not present\n");
      wlk->type  = PG_WALK_TYPE_PML4E;
      wlk->entry = (void*)pml4e;
      return VM_FAULT;
   }

   pdp = (pdpe_t*)page_addr(pml4e->addr);
   if(vmm_area_range(pdp, PG_4K_SIZE))
   {
      debug(PG_WLK, "pdp in vmm area\n");
      return VM_FAIL;
   }

   pdpe = &pdp[pdp_idx(vaddr)];
   debug(PG_WLK, "pdpe @ 0x%X = 0x%X\n", (offset_t)pdpe, pdpe->raw);

   if(!pg_present(pdpe))
   {
      debug(PG_WLK, "pdpe not present\n");
      wlk->type  = PG_WALK_TYPE_PDPE;
      wlk->entry = (void*)pdpe;
      return VM_FAULT;
   }

   if(info->vmm.cpu.skillz.pg_1G && pg_large(pdpe))
   {
      wlk->addr  = pg_1G_addr((offset_t)pdpe->page.addr) + pg_1G_offset(vaddr);
      wlk->type  = PG_WALK_TYPE_PDPE;
      wlk->size  = PG_1G_SIZE;
      wlk->entry = (void*)pdpe;
      goto __success;
   }

   pd = (pde64_t*)page_addr(pdpe->addr);
   if(vmm_area_range(pd, PG_4K_SIZE))
   {
      debug(PG_WLK, "pd64 in vmm area\n");
      return VM_FAIL;
   }

   pde = &pd[pd64_idx(vaddr)];
   debug(PG_WLK, "pde64 @ 0x%X = 0x%X\n", (offset_t)pde, pde->raw);

   if(!pg_present(pde))
   {
      debug(PG_WLK, "pde not present\n");
      wlk->type  = PG_WALK_TYPE_PDE64;
      wlk->entry = (void*)pde;
      return VM_FAULT;
   }

   if(pg_large(pde))
   {
      wlk->addr  = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      wlk->type  = PG_WALK_TYPE_PDE64;
      wlk->size  = PG_2M_SIZE;
      wlk->entry = (void*)pde;
      goto __success;
   }

   pt = (pte64_t*)page_addr(pde->addr);
   if(vmm_area_range(pt, PG_4K_SIZE))
   {
      debug(PG_WLK, "pt64 in vmm area\n");
      return VM_FAIL;
   }

   pte = &pt[pt64_idx(vaddr)];
   debug(PG_WLK, "pte64 @ 0x%X = 0x%X\n", (offset_t)pte, pte->raw);

   if(!pg_present(pte))
   {
      debug(PG_WLK, "pte not present\n");
      wlk->type  = PG_WALK_TYPE_PTE64;
      wlk->entry = (void*)pte;
      return VM_FAULT;
   }

   wlk->addr  = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);
   wlk->type  = PG_WALK_TYPE_PTE64;
   wlk->size  = PG_4K_SIZE;
   wlk->entry = (void*)pte;

__success:
   debug(PG_WLK, "lmode vaddr 0x%X -> guest paddr 0x%X\n", vaddr, wlk->addr);
   return VM_DONE;
}

/*
** pmode+pae: 2MB and 4KB pages
** cr4.pse is used
*/
static int __pg_walk_pmode_pae(cr3_reg_t *cr3, offset_t _vaddr, pg_wlk_t *wlk)
{
   pdpe_t   *pdp, *pdpe;
   pde64_t  *pd, *pde;
   pte64_t  *pt, *pte;
   uint32_t vaddr = _vaddr & 0xffffffff;

   pdp = (pdpe_t*)pg_32B_addr((offset_t)cr3->pae.addr);
   if(vmm_area_range(pdp, PG_4K_SIZE))
   {
      debug(PG_WLK, "pdp_pae in vmm area\n");
      return VM_FAIL;
   }

   pdpe = &pdp[pdp_pae_idx(vaddr)];
   debug(PG_WLK, "pdpe_pae @ 0x%X = 0x%X\n", (offset_t)pdpe, pdpe->raw);

   if(!pg_present(pdpe))
   {
      debug(PG_WLK, "pdpe_pae not present\n");
      wlk->type  = PG_WALK_TYPE_PDPE_PAE;
      wlk->entry = (void*)pdpe;
      return VM_FAULT;
   }

   pd = (pde64_t*)page_addr(pdpe->pae.addr);
   if(vmm_area_range(pd, PG_4K_SIZE))
   {
      debug(PG_WLK, "pd64 in vmm area\n");
      return VM_FAIL;
   }

   pde = &pd[pd64_idx(vaddr)];
   debug(PG_WLK, "pde64 @ 0x%X = 0x%X\n", (offset_t)pde, pde->raw);

   if(!pg_present(pde))
   {
      debug(PG_WLK, "pde not present\n");
      wlk->type  = PG_WALK_TYPE_PDE64;
      wlk->entry = (void*)pde;
      return VM_FAULT;
   }

   if(__cr4.pse && pg_large(pde))
   {
      wlk->addr  = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      wlk->type  = PG_WALK_TYPE_PDE64;
      wlk->size  = PG_2M_SIZE;
      wlk->entry = (void*)pde;
      goto __success;
   }

   pt = (pte64_t*)page_addr(pde->addr);
   if(vmm_area_range(pt, PG_4K_SIZE))
   {
      debug(PG_WLK, "pt64 in vmm area\n");
      return VM_FAIL;
   }

   pte = &pt[pt64_idx(vaddr)];
   debug(PG_WLK, "pte64 @ 0x%X = 0x%X\n", (offset_t)pte, pte->raw);

   if(!pg_present(pte))
   {
      debug(PG_WLK, "pte64 not present\n");
      wlk->type  = PG_WALK_TYPE_PTE64;
      wlk->entry = (void*)pte;
      return VM_FAULT;
   }

   wlk->addr  = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);
   wlk->type  = PG_WALK_TYPE_PTE64;
   wlk->size  = PG_4K_SIZE;
   wlk->entry = (void*)pte;

__success:
   debug(PG_WLK, "pae vaddr 0x%x -> guest paddr 0x%X\n", vaddr, wlk->addr);
   return VM_DONE;
}

/*
** pmode: 4MB and 4KB pages
** cr4.pse is used
*/
static int __pg_walk_pmode(cr3_reg_t *cr3, offset_t _vaddr, pg_wlk_t *wlk)
{
   pde32_t  *pd, *pde;
   pte32_t  *pt, *pte;
   uint32_t vaddr = _vaddr & 0xffffffff;

   pd = (pde32_t*)page_addr(cr3->addr);
   if(vmm_area_range(pd, PG_4K_SIZE))
   {
      debug(PG_WLK, "pd32 in vmm area\n");
      return VM_FAIL;
   }

   pde = &pd[pd32_idx(vaddr)];
   debug(PG_WLK, "pde32 @ 0x%X = 0x%x\n", (offset_t)pde, pde->raw);

   if(!pg_present(pde))
   {
      debug(PG_WLK, "pde32 not present\n");
      wlk->type  = PG_WALK_TYPE_PDE32;
      wlk->entry = (void*)pde;
      return VM_FAULT;
   }

   if(__cr4.pse && pg_large(pde))
   {
      wlk->addr  = pg_4M_addr((uint32_t)pde->page.addr) + pg_4M_offset(vaddr);
      wlk->type  = PG_WALK_TYPE_PDE32;
      wlk->size  = PG_4M_SIZE;
      wlk->entry = (void*)pde;
      goto __success;
   }

   pt = (pte32_t*)page_addr(pde->addr);
   if(vmm_area_range(pt, PG_4K_SIZE))
   {
      debug(PG_WLK, "pt32 in vmm area\n");
      return VM_FAIL;
   }

   pte = &pt[pt32_idx(vaddr)];
   debug(PG_WLK, "pte32 @ 0x%X = 0x%x\n", (offset_t)pte, pte->raw);

   if(!pg_present(pte))
   {
      debug(PG_WLK, "pte32 not present\n");
      wlk->type  = PG_WALK_TYPE_PTE32;
      wlk->entry = (void*)pte;
      return VM_FAULT;
   }

   wlk->addr  = (offset_t)(pg_4K_addr((uint32_t)pte->addr) + pg_4K_offset(vaddr));
   wlk->type  = PG_WALK_TYPE_PTE32;
   wlk->size  = PG_4K_SIZE;
   wlk->entry = (void*)pte;

__success:
   debug(PG_WLK, "pmode vaddr 0x%x -> guest paddr 0x%X\n", vaddr, wlk->addr);
   return VM_DONE;
}

/*
** Page walking service
**
** . we use VMM cpu skillz as we depend
**   upon system mmu and not nested one
**
** . we support :
**   - long/compatibility mode
**   - legacy protected mode + paging + pae
**   - legacy protected mode + paging
*/
int __pg_walk(cr3_reg_t *cr3, offset_t vaddr, pg_wlk_t *wlk)
{
   debug(PG_WLK, "cr3 0x%X\n", cr3->raw);

   if(_xx_lmode())
      return __pg_walk_lmode(cr3, vaddr, wlk);

   if(__cr4.pae)
      return __pg_walk_pmode_pae(cr3, vaddr, wlk);

   return __pg_walk_pmode(cr3, vaddr, wlk);
}
