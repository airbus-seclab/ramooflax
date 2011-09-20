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
#include <pool.h>
#include <paging.h>
#include <string.h>
#include <info_data.h>

#ifndef __INIT__
#include <debug.h>
#endif

extern info_data_t *info;

/* Dynamic allocation failures are fatal ! */
static inline offset_t __pg_new_pg()
{
   offset_t pg = pool_pop_page();

   if(!pg)
      panic("pool_pop_page()");

   memset((void*)pg, 0, PAGE_SIZE);
   return pg;
}

static inline pde64_t* __pg_new_pd()
{
   return (pde64_t*)__pg_new_pg();
}

static inline pte64_t* __pg_new_pt()
{
   return (pte64_t*)__pg_new_pg();
}

static inline pdpe_t* __pg_get_pdpe_nocheck(offset_t addr)
{
   return &info->vm.cpu.pg.pm.pdp[pml4_idx(addr)][pdp_idx(addr)];
}

static inline pdpe_t* __pg_get_pdpe(offset_t addr)
{
   pml4e_t *pml4e = &info->vm.cpu.pg.pm.pml4[pml4_idx(addr)];

   if(!pml4e->p)
      return 0;

   return __pg_get_pdpe_nocheck(addr);
}

static inline pde64_t* __pg_get_pde_nocheck(pdpe_t *pdpe, offset_t addr)
{
   pde64_t *pd = (pde64_t*)page_addr(pdpe->addr);
   return &pd[pd64_idx(addr)];
}

static inline pde64_t* __pg_get_pde(pdpe_t *pdpe, offset_t addr)
{
   if(!pdpe->p || pdpe->page.ps)
      return 0;

   return __pg_get_pde_nocheck(pdpe, addr);
}

static inline pte64_t* __pg_get_pte_nocheck(pde64_t *pde, offset_t addr)
{
   pte64_t *pt = (pte64_t*)page_addr(pde->addr);
   return &pt[pt64_idx(addr)];
}

static inline pte64_t* __pg_get_pte(pde64_t *pde, offset_t addr)
{
   if(!pde->p || pde->page.ps)
      return 0;

   return __pg_get_pte_nocheck(pde, addr);
}

/* resolve pdpe (pdp tables are pre-allocated) */
static inline pdpe_t* __pg_resolve_pdpe(offset_t addr, uint32_t pvl)
{
   pml4e_t *pml4e;
   pdpe_t  *pdp;

   pml4e = &info->vm.cpu.pg.pm.pml4[pml4_idx(addr)];
   pdp   = info->vm.cpu.pg.pm.pdp[pml4_idx(addr)];

   if(!pml4e->p)
      __pg_set_entry(pml4e, pvl, page_nr(pdp));

   return &pdp[pdp_idx(addr)];
}

/*
** resolve pde (allocate pd if needed) for non-large pdpe
** return 0 if large page
*/
static inline pde64_t* __pg_resolve_pde(pdpe_t *pdpe, offset_t addr, uint32_t pvl)
{
   pde64_t *pd;

   if(!pdpe->p)
   {
      pd = __pg_new_pd();
      __pg_set_entry(pdpe, pvl, page_nr(pd));
   }
   else if(pdpe->page.ps)
      return 0;
   else
      pd = (pde64_t*)page_addr(pdpe->addr);

   return &pd[pd64_idx(addr)];
}

/*
** resolve pte (allocate pt if needed for non-large pde
** return 0 if large page
*/
static inline pte64_t* __pg_resolve_pte(pde64_t *pde, offset_t addr, uint32_t pvl)
{
   pte64_t *pt;

   if(!pde->p)
   {
      pt = __pg_new_pt();
      __pg_set_entry(pde, pvl, page_nr(pt));
   }
   else if(pde->page.ps)
      return 0;
   else
      pt = (pte64_t*)page_addr(pde->addr);

   return &pt[pt64_idx(addr)];
}

static void __pg_unmap_2M(pde64_t *pde)
{
   pte64_t  *pt;
   uint32_t i;

   if(!pde->page.ps)
   {
      pt = (pte64_t*)page_addr(pde->addr);

      for(i=0 ; i<PTE64_PER_PT ; i++)
	 if(pt[i].p)
	    pt[i].p = 0;

      pool_push_page((offset_t)pt);
   }

   pde->p = 0;
}

static void __pg_map_2M_nolarge(pde64_t *pde, offset_t addr, uint32_t pvl)
{
   pte64_t  *pt;
   offset_t pfn;
   uint32_t i;

   pt  = __pg_new_pt();
   pfn = pg_4K_nr(addr);
   for(i=0 ; i<PTE64_PER_PT ; i++, pfn++)
      __pg_set_entry(&pt[i], pvl, pfn);

   __pg_set_entry(pde, pvl, page_nr(pt));
}

static void __pg_map_2M(pde64_t *pde, offset_t addr, uint32_t pvl)
{
   if(pde->p)
      __pg_unmap_2M(pde);

   if(info->vm.cpu.skillz.pg_2M)
   {
      __pg_set_large_entry(pde, pvl, pg_2M_nr(addr));
      return;
   }

   __pg_map_2M_nolarge(pde, addr, pvl);
}

static void __pg_remap_2M(pde64_t *pde, offset_t addr)
{
   uint32_t pvl = __pg_get_pvl(pde);
   __pg_unmap_2M(pde);
   __pg_map_2M_nolarge(pde, addr, pvl);
}

static void __pg_unmap_1G(pdpe_t *pdpe)
{
   pde64_t  *pd;
   uint32_t i;

   if(!pdpe->page.ps)
   {
      pd = (pde64_t*)page_addr(pdpe->addr);

      for(i=0 ; i<PDE64_PER_PD ; i++)
	 if(pd[i].p)
	    __pg_unmap_2M(&pd[i]);

      pool_push_page((offset_t)pd);
   }

   pdpe->p = 0;
}

static void __pg_map_1G_nolarge(pdpe_t *pdpe, offset_t addr, uint32_t pvl)
{
   pde64_t  *pd;
   uint32_t i;

   pd = __pg_new_pd();
   for(i=0 ; i<PDE64_PER_PD ; i++)
   {
      __pg_map_2M(&pd[i], addr, pvl);
      addr += PG_2M_SIZE;
   }

   __pg_set_entry(pdpe, pvl, page_nr(pd));
}

static void __pg_map_1G(pdpe_t *pdpe, offset_t addr, uint32_t pvl)
{
   if(pdpe->p)
      __pg_unmap_1G(pdpe);

   if(info->vm.cpu.skillz.pg_1G)
   {
      __pg_set_large_entry(pdpe, pvl, pg_1G_nr(addr));
      return;
   }

   __pg_map_1G_nolarge(pdpe, addr, pvl);
}

static void __pg_remap_1G(pdpe_t *pdpe, offset_t addr)
{
   uint32_t pvl = __pg_get_pvl(pdpe);
   __pg_unmap_1G(pdpe);
   __pg_map_1G_nolarge(pdpe, addr, pvl);
}

/*
** Range mapping
*/
static inline offset_t
__pg_map_4K(offset_t start, offset_t end, offset_t upper, uint32_t pvl)
{
   offset_t addr = pg_4K_align(start);

   while(addr < min(pg_4K_align(end), upper))
   {
      pg_map_4K(addr, pvl);
      addr += PG_4K_SIZE;
   }

   return addr;
}

static inline offset_t
__pg_map_bk_2M(offset_t start, offset_t end, offset_t upper, uint32_t pvl)
{
   offset_t addr, start_2M_up;
   bool_t   diff_pt;

   start_2M_up = pg_2M_align_next(start);
   diff_pt = (pt64_nr(start) != pt64_nr(end));

   if(pg_2M_aligned(start) && diff_pt)
   {
      pg_map_2M(start, pvl);
      addr = start_2M_up;
   }
   else
      addr = __pg_map_4K(start, end, start_2M_up, pvl);

   while(addr < min(pg_2M_align(end), upper))
   {
      pg_map_2M(addr, pvl);
      addr += PG_2M_SIZE;
   }

   return addr;
}

static inline offset_t
__pg_map_bk_1G(offset_t start, offset_t end, offset_t upper, uint32_t pvl)
{
   offset_t addr, start_1G_up;
   bool_t   diff_pd;

   start_1G_up = pg_1G_align_next(start);
   diff_pd = (pd64_nr(start) != pd64_nr(end));

   if(pg_1G_aligned(start) && diff_pd)
   {
      pg_map_1G(start, pvl);
      addr = start_1G_up;
   }
   else
      addr = __pg_map_bk_2M(start, end, start_1G_up, pvl);

   while(addr < min(pg_1G_align(end), upper))
   {
      pg_map_1G(addr, pvl);
      addr += PG_1G_SIZE;
   }

   return addr;
}

static inline offset_t
__pg_map_bk_512G(offset_t start, offset_t end, offset_t upper, uint32_t pvl)
{
   offset_t addr, start_512G_up;
   bool_t   diff_pdp;

   start_512G_up = pg_512G_align_next(start);
   diff_pdp = (pdp_nr(start) != pdp_nr(end));

   if(pg_512G_aligned(start) && diff_pdp)
   {
      pg_map_512G(start, pvl);
      addr = start_512G_up;
   }
   else
      addr = __pg_map_bk_1G(start, end, start_512G_up, pvl);

   /* should be min(upper, MAX_VADDR) */
   while(addr < upper)
   {
      pg_map_512G(addr, pvl);
      addr += PG_512G_SIZE;
   }

   return addr;
}

static inline void __pg_map_fw_2M(offset_t start, offset_t end, uint32_t pvl)
{
   offset_t addr = start; /* do not align */

   while(addr < pg_2M_align(end))
   {
      pg_map_2M(addr, pvl);
      addr += PG_2M_SIZE;
   }

   if(!pg_2M_aligned(end))
      __pg_map_4K(start, end, end, pvl);
}

static inline void __pg_map_fw_1G(offset_t start, offset_t end, uint32_t pvl)
{
   offset_t addr = start; /* do not align */

   while(addr < pg_1G_align(end))
   {
      pg_map_1G(addr, pvl);
      addr += PG_1G_SIZE;
   }

   if(!pg_1G_aligned(end))
      __pg_map_fw_2M(addr, end, pvl);
}

static inline void __pg_map_fw_512G(offset_t start, offset_t end, uint32_t pvl)
{
   offset_t addr = start; /* do not align */

   while(addr < pg_512G_align(end))
   {
      pg_map_512G(addr, pvl);
      addr += PG_512G_SIZE;
   }

   if(!pg_512G_aligned(end))
      __pg_map_fw_1G(addr, end, pvl);
}

static offset_t __pg_map_backward(offset_t start, offset_t end, uint32_t pvl)
{
   return __pg_map_bk_512G(start, end, pg_512G_align(end),pvl);
}

static void __pg_map_forward(offset_t start, offset_t end, uint32_t pvl)
{
   __pg_map_fw_512G(start, end, pvl);
}

/*
** Mapping services:
**
** - pg_map_xxx() maps xxx bytes from
**   an xxx ALIGNED addr using pvl privileges
**
** - pg_map(x,y) maps [x;y[
*/
void pg_map_512G(offset_t addr, uint32_t pvl)
{
   pdpe_t   *pdp;
   uint32_t i;

   pdp = __pg_resolve_pdpe(pg_512G_align(addr), pvl);

   for(i=0 ; i<PDPE_PER_PDP ; i++)
   {
      __pg_map_1G(&pdp[i], addr, pvl);
      addr += PG_1G_SIZE;
   }
}

void pg_map_1G(offset_t addr, uint32_t pvl)
{
   pdpe_t *pdpe = __pg_resolve_pdpe(addr, pvl);
   __pg_map_1G(pdpe, addr, pvl);
}

void pg_map_2M(offset_t addr, uint32_t pvl)
{
   pdpe_t  *pdpe = __pg_resolve_pdpe(addr, pvl);
   pde64_t *pde;

   /* already 1GB mapping */
   if(pdpe->page.ps)
      return;

   pde = __pg_resolve_pde(pdpe, addr, pvl);
   __pg_map_2M(pde, addr, pvl);
}

void pg_map_4K(offset_t addr, uint32_t pvl)
{
   pdpe_t  *pdpe = __pg_resolve_pdpe(addr, pvl);
   pde64_t *pde  = __pg_resolve_pde(pdpe, addr, pvl);
   pte64_t *pte;

   /* already 1GB or 2MB mapping */
   if(pdpe->page.ps || pde->page.ps)
      return;

   pte = __pg_resolve_pte(pde, addr, pvl);
   __pg_set_entry(pte, pvl, page_nr(addr));
}

void pg_map(offset_t start, offset_t end, uint32_t pvl)
{
   __pg_map_forward(__pg_map_backward(start, end, pvl), end, pvl);
}

/*
** Unmapping services (cf. above)
** beware that addr must be ALIGNED on xxx bytes
*/
void pg_unmap_512G(offset_t addr)
{
   pdpe_t   *pdp = __pg_get_pdpe(pg_512G_align(addr));
   uint32_t i;

   if(!pdp)
      return;

   for(i=0 ; i<PDPE_PER_PDP ; i++)
      if(pdp[i].p)
	 __pg_unmap_1G(&pdp[i]);
}

void pg_unmap_1G(offset_t addr)
{
   pdpe_t *pdpe = __pg_get_pdpe(addr);

   if(pdpe && pdpe->p)
      __pg_unmap_1G(pdpe);
}

void pg_unmap_2M(offset_t addr)
{
   pdpe_t  *pdpe = __pg_get_pdpe(addr);
   pde64_t *pde;

   if(!pdpe || !pdpe->p)
      return;

   if(pdpe->page.ps)
      __pg_remap_1G(pdpe, addr);

   pde = __pg_get_pde(pdpe, addr);
   __pg_unmap_2M(pde);
}

void pg_unmap_4K(offset_t addr)
{
   pdpe_t  *pdpe = __pg_get_pdpe(addr);
   pde64_t *pde;
   pte64_t *pte;

   if(!pdpe || !pdpe->p)
      return;

   if(pdpe->page.ps)
      __pg_remap_1G(pdpe, addr);

   pde = __pg_get_pde(pdpe, addr);

   if(!pde || !pde->p)
      return;

   if(pde->page.ps)
      __pg_remap_2M(pde, addr);

   pte = __pg_get_pte(pde, addr);
   pte->p = 0;
}

#ifndef __INIT__
/*
** VM page walking services using VMM cpu skillz (real cr3)
*/

/*
** lmode: 1GB, 2MB and 4KB pages
** cr4.pse is ignored
** 1GB cpuid feature must be checked
*/
static inline int pg_walk_lmode(cr3_reg_t *cr3, offset_t vaddr, offset_t *paddr, size_t *psz)
{
   pml4e_t *pml4, *pml4e;
   pdpe_t  *pdp, *pdpe;
   pde64_t *pd, *pde;
   pte64_t *pt, *pte;

   pml4 = (pml4e_t*)page_addr(cr3->pml4.addr);
   if(vmm_area(pml4))
   {
      debug(PG, "pml4 in vmm area\n");
      return 0;
   }

   pml4e = &pml4[pml4_idx(vaddr)];
   if(!pml4e->p)
   {
      debug(PG, "pml4e not present\n");
      return 0;
   }

   pdp = (pdpe_t*)page_addr(pml4e->addr);
   if(vmm_area(pdp))
   {
      debug(PG, "pdp in vmm area\n");
      return 0;
   }

   pdpe = &pdp[pdp_idx(vaddr)];
   if(!pdpe->p)
   {
      debug(PG, "pdpe not present\n");
      return 0;
   }

   if(info->vmm.cpu.skillz.pg_1G && pdpe->page.ps)
   {
      *paddr = pg_1G_addr((offset_t)pdpe->page.addr) + pg_1G_offset(vaddr);
      *psz = PG_1G_SIZE;
      goto __prepare_addr;
   }

   pd = (pde64_t*)page_addr(pdpe->addr);
   if(vmm_area(pd))
   {
      debug(PG, "pd in vmm area\n");
      return 0;
   }

   pde = &pd[pd64_idx(vaddr)];
   if(!pde->p)
   {
      debug(PG, "pde not present\n");
      return 0;
   }

   if(pde->page.ps)
   {
      *paddr = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      *psz = PG_2M_SIZE;
      goto __prepare_addr;
   }

   pt = (pte64_t*)page_addr(pde->addr);
   if(vmm_area(pt))
   {
      debug(PG, "pt in vmm area\n");
      return 0;
   }

   pte = &pt[pt64_idx(vaddr)];
   if(!pte->p)
   {
      debug(PG, "pte not present\n");
      return 0;
   }

   *paddr = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);
   *psz = PG_4K_SIZE;

__prepare_addr:
   if(vmm_area(*paddr))
      return 0;

   debug(PG, "lmode vaddr 0x%X -> paddr 0x%X\n", vaddr, *paddr);
   return 1;
}

/*
** pmode+pae: 2MB and 4KB pages
** cr4.pse is used
*/
static inline int pg_walk_pmode_pae(cr3_reg_t *cr3, offset_t _vaddr, offset_t *paddr, size_t *psz)
{
   pdpe_t   *pdp, *pdpe;
   pde64_t  *pd, *pde;
   pte64_t  *pt, *pte;
   uint32_t vaddr = _vaddr & 0xffffffff;

   pdp = (pdpe_t*)pg_32B_addr((offset_t)cr3->pae.addr);
   if(vmm_area(pdp))
      return 0;

   pdpe = &pdp[pdp_pae_idx(vaddr)];
   if(!pdpe->p)
      return 0;

   pd = (pde64_t*)page_addr(pdpe->addr);
   if(vmm_area(pd))
      return 0;

   pde = &pd[pd64_idx(vaddr)];
   if(!pde->p)
      return 0;

   if(__cr4.pse && pde->page.ps)
   {
      *paddr = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      *psz = PG_2M_SIZE;
      goto __prepare_addr;
   }

   pt = (pte64_t*)page_addr(pde->addr);
   if(vmm_area(pt))
      return 0;

   pte = &pt[pt64_idx(vaddr)];
   if(!pte->p)
      return 0;

   *paddr = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);
   *psz = PG_4K_SIZE;

__prepare_addr:
   if(vmm_area(*paddr))
      return 0;

   debug(PG, "pae vaddr 0x%x -> paddr 0x%x\n", vaddr, paddr);
   return 1;
}

/*
** pmode: 4MB and 4KB pages
** cr4.pse is used
*/
static inline int pg_walk_pmode(cr3_reg_t *cr3, offset_t _vaddr, offset_t *_paddr, size_t *psz)
{
   pde32_t  *pd, *pde;
   pte32_t  *pt, *pte;
   uint32_t paddr;
   uint32_t vaddr = _vaddr & 0xffffffff;

   pd = (pde32_t*)page_addr(cr3->addr);
   if(vmm_area(pd))
   {
      debug(PG, "pd in vmm area\n");
      return 0;
   }

   pde = &pd[pd32_idx(vaddr)];
   debug(PG, "pde @ 0x%X = 0x%x\n", (offset_t)pde, pde->raw);
   if(!pde->p)
   {
      debug(PG, "pde not present\n");
      return 0;
   }

   if(__cr4.pse && pde->page.ps)
   {
      debug(PG, "large page\n");
      paddr = pg_4M_addr((uint32_t)pde->page.addr) + pg_4M_offset(vaddr);
      *psz = PG_4M_SIZE;
      goto __prepare_addr;
   }

   pt = (pte32_t*)page_addr(pde->addr);
   if(vmm_area(pt))
   {
      debug(PG, "pt in vmm area\n");
      return 0;
   }

   pte = &pt[pt32_idx(vaddr)];
   debug(PG, "pte @ 0x%X = 0x%x\n", (offset_t)pte, pte->raw);
   if(!pte->p)
   {
      debug(PG, "pte not present\n");
      return 0;
   }

   paddr = pg_4K_addr((uint32_t)pte->addr) + pg_4K_offset(vaddr);
   *psz = PG_4K_SIZE;

__prepare_addr:
   if(vmm_area(paddr))
      return 0;

   debug(PG, "pmode vaddr 0x%x -> paddr 0x%x\n", vaddr, paddr);
   *_paddr = (offset_t)paddr;
   return 1;
}

/*
** . we resolve guest virtual into guest physical
**   using guest paging structures NOT nested ones
**
** . guest can be in :
**   - long/compatibility mode
**   - legacy protected mode + paging + pae
**   - legacy protected mode + paging
*/
int __pg_walk(cr3_reg_t *cr3, offset_t vaddr, offset_t *paddr, size_t *psz)
{
   if(_xx_lmode())
      return pg_walk_lmode(cr3, vaddr, paddr, psz);

   if(__cr4.pae)
      return pg_walk_pmode_pae(cr3, vaddr, paddr, psz);

   return pg_walk_pmode(cr3, vaddr, paddr, psz);
}

int pg_walk(offset_t vaddr, offset_t *paddr, size_t *psz)
{
   if(!__paging())
   {
      debug(PG, "paging disabled !\n");
      return 0;
   }

   return __pg_walk(&__cr3, vaddr, paddr, psz);
}

/*
** walk VM nested page tables using VM cpu skillz (nested features)
*/
int pg_nested_walk(offset_t vaddr, offset_t *paddr)
{
   pml4e_t *pml4e;
   pdpe_t  *pdpe;
   pde64_t *pde;
   pte64_t *pte;

   debug(PG, "nCR3: 0x%X\n", __ncr3.raw);

   pml4e = &info->vm.cpu.pg.pm.pml4[pml4_idx(vaddr)];

   debug(PG, "pml4e @ 0x%X = 0x%X\n", (offset_t)pml4e, pml4e->raw);
   if(!pml4e->p)
   {
      debug(PG, "pml4e not present\n");
      return 0;
   }

   if(__rmode())
      pdpe = &info->vm.cpu.pg.rm->pdp[pdp_idx(vaddr)];
   else
      pdpe = &info->vm.cpu.pg.pm.pdp[pml4_idx(vaddr)][pdp_idx(vaddr)];

   debug(PG, "pdpe @ 0x%X = 0x%X\n", (offset_t)pdpe, pdpe->raw);
   if(!pdpe->p)
   {
      debug(PG, "pdpe not present\n");
      return 0;
   }

   if(info->vm.cpu.skillz.pg_1G && pdpe->page.ps)
   {
      *paddr = pg_1G_addr((offset_t)pdpe->page.addr) + pg_1G_offset(vaddr);
      goto __success;
   }

   pde = __pg_get_pde_nocheck(pdpe, vaddr);

   debug(PG, "pde @ 0x%X = 0x%X\n", (offset_t)pde, pde->raw);
   if(!pde->p)
   {
      debug(PG, "pde not present\n");
      return 0;
   }

   if(info->vm.cpu.skillz.pg_2M && pde->page.ps)
   {
      *paddr = pg_2M_addr((offset_t)pde->page.addr) + pg_2M_offset(vaddr);
      goto __success;
   }

   pte = __pg_get_pte_nocheck(pde, vaddr);

   debug(PG, "pte @ 0x%X = 0x%X\n", (offset_t)pte, pte->raw);
   if(!pte->p)
   {
      debug(PG, "pte not present\n");
      return 0;
   }

   *paddr = pg_4K_addr((offset_t)pte->addr) + pg_4K_offset(vaddr);

__success:
   debug(PG, "paddr 0x%x -> nested addr 0x%x\n", vaddr, *paddr);
   return 1;
}

void pg_show(offset_t vaddr)
{
   size_t   psz;
   offset_t paddr;
   offset_t naddr;

   if(pg_walk(vaddr, &paddr, &psz))
      pg_nested_walk(paddr, &naddr);
}



#endif
