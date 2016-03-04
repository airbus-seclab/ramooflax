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
** lmode: 1GB, 2MB and 4KB pages
** cr4.pse is ignored
** 1GB cpuid feature must be checked
*/
static inline
int pg_walk_lmode(cr3_reg_t *cr3,
		  offset_t vaddr, offset_t *paddr,
		  size_t *psz, int chk)
{
   pml4e_t *pml4, *pml4e;
   pdpe_t  *pdp, *pdpe;
   pde64_t *pd, *pde;
   pte64_t *pt, *pte;

   pml4 = (pml4e_t*)page_addr(cr3->pml4.addr);
   if(chk && vmm_area(pml4))
   {
      debug(PG_WLK, "pml4 in vmm area\n");
      return 0;
   }

   pml4e = &pml4[pml4_idx(vaddr)];
   debug(PG_WLK, "pml4e @ 0x%X = %X\n", (offset_t)pml4e, pml4e->raw);

   if(!pg_present(pml4e))
   {
      debug(PG_WLK, "pml4e not present\n");
      return 0;
   }

   pdp = (pdpe_t*)page_addr(pml4e->addr);
   if(chk && vmm_area(pdp))
   {
      debug(PG_WLK, "pdp in vmm area\n");
      return 0;
   }

   pdpe = &pdp[pdp_idx(vaddr)];
   debug(PG_WLK, "pdpe @ 0x%X = 0x%X\n", (offset_t)pdpe, pdpe->raw);

   if(!pg_present(pdpe))
   {
      debug(PG_WLK, "pdpe not present\n");
      return 0;
   }

   if(info->vmm.cpu.skillz.pg_1G && pg_large(pdpe))
   {
      *paddr = pg_1G_addr((offset_t)pdpe->page.addr) + pg_1G_offset(vaddr);
      *psz = PG_1G_SIZE;
      goto __prepare_addr;
   }

   pd = (pde64_t*)page_addr(pdpe->addr);
   if(chk && vmm_area(pd))
   {
      debug(PG_WLK, "pd in vmm area\n");
      return 0;
   }

   pde = &pd[pd64_idx(vaddr)];
   debug(PG_WLK, "pde @ 0x%X = 0x%X\n", (offset_t)pde, pde->raw);

   if(!pg_present(pde))
   {
      debug(PG_WLK, "pde not present\n");
      return 0;
   }

   if(pg_large(pde))
   {
      *paddr = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      *psz = PG_2M_SIZE;
      goto __prepare_addr;
   }

   pt = (pte64_t*)page_addr(pde->addr);
   if(chk && vmm_area(pt))
   {
      debug(PG_WLK, "pt in vmm area\n");
      return 0;
   }

   pte = &pt[pt64_idx(vaddr)];
   debug(PG_WLK, "pte @ 0x%X = 0x%X\n", (offset_t)pte, pte->raw);

   if(!pg_present(pte))
   {
      debug(PG_WLK, "pte not present\n");
      return 0;
   }

   *paddr = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);
   *psz = PG_4K_SIZE;

__prepare_addr:
   if(chk && vmm_area(*paddr))
   {
      debug(PG_WLK, "paddr 0x%x in vmm area\n", *paddr);
      return 0;
   }

   debug(PG_WLK, "lmode vaddr 0x%X -> paddr 0x%X\n", vaddr, *paddr);
   return 1;
}

/*
** pmode+pae: 2MB and 4KB pages
** cr4.pse is used
*/
static inline
int pg_walk_pmode_pae(cr3_reg_t *cr3,
		      offset_t _vaddr, offset_t *paddr,
		      size_t *psz, int chk)
{
   pdpe_t   *pdp, *pdpe;
   pde64_t  *pd, *pde;
   pte64_t  *pt, *pte;
   uint32_t vaddr = _vaddr & 0xffffffff;

   pdp = (pdpe_t*)pg_32B_addr((offset_t)cr3->pae.addr);
   if(chk && vmm_area(pdp))
   {
      debug(PG_WLK, "pdp in vmm area\n");
      return 0;
   }

   pdpe = &pdp[pdp_pae_idx(vaddr)];
   debug(PG_WLK, "pdpe @ 0x%X = 0x%X\n", (offset_t)pdpe, pdpe->raw);

   if(!pg_present(pdpe))
   {
      debug(PG_WLK, "pdpe not present\n");
      return 0;
   }

   pd = (pde64_t*)page_addr(pdpe->pae.addr);
   if(chk && vmm_area(pd))
   {
      debug(PG_WLK, "pd in vmm area\n");
      return 0;
   }

   pde = &pd[pd64_idx(vaddr)];
   debug(PG_WLK, "pde @ 0x%X = 0x%X\n", (offset_t)pde, pde->raw);

   if(!pg_present(pde))
   {
      debug(PG_WLK, "pde not present\n");
      return 0;
   }

   if(__cr4.pse && pg_large(pde))
   {
      *paddr = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      *psz = PG_2M_SIZE;
      goto __prepare_addr;
   }

   pt = (pte64_t*)page_addr(pde->addr);
   if(chk && vmm_area(pt))
   {
      debug(PG_WLK, "pt in vmm area\n");
      return 0;
   }

   pte = &pt[pt64_idx(vaddr)];
   debug(PG_WLK, "pte @ 0x%X = 0x%X\n", (offset_t)pte, pte->raw);

   if(!pg_present(pte))
   {
      debug(PG_WLK, "pte not present\n");
      return 0;
   }

   *paddr = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);
   *psz = PG_4K_SIZE;

__prepare_addr:
   if(chk && vmm_area(*paddr))
   {
      debug(PG_WLK, "paddr 0x%x in vmm area\n", *paddr);
      return 0;
   }

   debug(PG_WLK, "pae vaddr 0x%x -> paddr 0x%x\n", vaddr, *paddr);
   return 1;
}

/*
** pmode: 4MB and 4KB pages
** cr4.pse is used
*/
static inline
int pg_walk_pmode(cr3_reg_t *cr3,
		  offset_t _vaddr, offset_t *_paddr,
		  size_t *psz, int chk)
{
   pde32_t  *pd, *pde;
   pte32_t  *pt, *pte;
   uint32_t paddr;
   uint32_t vaddr = _vaddr & 0xffffffff;

   pd = (pde32_t*)page_addr(cr3->addr);
   if(chk && vmm_area(pd))
   {
      debug(PG_WLK, "pd in vmm area\n");
      return 0;
   }

   pde = &pd[pd32_idx(vaddr)];
   debug(PG_WLK, "pde @ 0x%X = 0x%x\n", (offset_t)pde, pde->raw);

   if(!pg_present(pde))
   {
      debug(PG_WLK, "pde not present\n");
      return 0;
   }

   if(__cr4.pse && pg_large(pde))
   {
      debug(PG_WLK, "large page found (pde->addr 0x%x)\n", (uint32_t)pde->page.addr);
      paddr = pg_4M_addr((uint32_t)pde->page.addr) + pg_4M_offset(vaddr);
      *psz = PG_4M_SIZE;
      goto __prepare_addr;
   }

   pt = (pte32_t*)page_addr(pde->addr);
   if(chk && vmm_area(pt))
   {
      debug(PG_WLK, "pt in vmm area\n");
      return 0;
   }

   pte = &pt[pt32_idx(vaddr)];
   debug(PG_WLK, "pte @ 0x%X = 0x%x\n", (offset_t)pte, pte->raw);

   if(!pg_present(pte))
   {
      debug(PG_WLK, "pte not present\n");
      return 0;
   }

   paddr = pg_4K_addr((uint32_t)pte->addr) + pg_4K_offset(vaddr);
   *psz = PG_4K_SIZE;

__prepare_addr:
   if(chk && vmm_area(paddr))
   {
      debug(PG_WLK, "paddr 0x%x in vmm area\n", paddr);
      return 0;
   }

   debug(PG_WLK, "pmode vaddr 0x%x -> paddr 0x%x\n", vaddr, paddr);
   *_paddr = (offset_t)paddr;
   return 1;
}

/*
** Page walking services
**
** . we use VMM cpu skillz as we depend
**   upon system mmu and not nested one
**
** . we support :
**   - long/compatibility mode
**   - legacy protected mode + paging + pae
**   - legacy protected mode + paging
*/
int __pg_walk(cr3_reg_t *cr3,
	      offset_t vaddr, offset_t *paddr,
	      size_t *psz, int chk)
{
   debug(PG_WLK, "cr3 0x%X\n", cr3->raw);

   if(_xx_lmode())
      return pg_walk_lmode(cr3, vaddr, paddr, psz, chk);

   if(__cr4.pae)
      return pg_walk_pmode_pae(cr3, vaddr, paddr, psz, chk);

   return pg_walk_pmode(cr3, vaddr, paddr, psz, chk);
}
