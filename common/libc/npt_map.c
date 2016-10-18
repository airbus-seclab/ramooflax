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
#include <pool.h>
#include <string.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** Allocations
*/
static inline offset_t __npg_new_pg()
{
   offset_t pg = pool_pop_page();

   if(!pg)
      panic("pool_pop_page()");

   memset((void*)pg, 0, PAGE_SIZE);
   return pg;
}

static inline npg_pdpe_t* __npg_new_pdp()
{
   debug(PG_MAP, "allocating new pdp\n");
   return (npg_pdpe_t*)__npg_new_pg();
}

static inline npg_pde64_t* __npg_new_pd()
{
   debug(PG_MAP, "allocating new pd\n");
   return (npg_pde64_t*)__npg_new_pg();
}

static inline npg_pte64_t* __npg_new_pt()
{
   debug(PG_MAP, "allocating new pt\n");
   return (npg_pte64_t*)__npg_new_pg();
}



/*
** Low level entry retrieval
*/
static inline npg_pdpe_t* __npg_get_pdpe_nocheck(npg_pml4e_t *pml4e, offset_t addr)
{
   npg_pdpe_t *pdp = (npg_pdpe_t*)page_addr(pml4e->addr);
   return &pdp[pdp_idx(addr)];
}

static inline npg_pde64_t* __npg_get_pde_nocheck(npg_pdpe_t *pdpe, offset_t addr)
{
   npg_pde64_t *pd = (npg_pde64_t*)page_addr(pdpe->addr);
   return &pd[pd64_idx(addr)];
}

static inline npg_pte64_t* __npg_get_pte_nocheck(npg_pde64_t *pde, offset_t addr)
{
   npg_pte64_t *pt = (npg_pte64_t*)page_addr(pde->addr);
   return &pt[pt64_idx(addr)];
}

static inline npg_pdpe_t* __npg_get_pdpe(offset_t addr)
{
   vm_pgmem_t  *pg = npg_get_active_paging();
   npg_pml4e_t *pml4e = &pg->pml4[pml4_idx(addr)];

   if(!npg_present(pml4e))
      return 0;

   return __npg_get_pdpe_nocheck(pml4e, addr);
}

static inline npg_pde64_t* __npg_get_pde(npg_pdpe_t *pdpe, offset_t addr)
{
   if(!npg_present(pdpe) || npg_large(pdpe))
      return 0;

   return __npg_get_pde_nocheck(pdpe, addr);
}

static inline npg_pte64_t* __npg_get_pte(npg_pde64_t *pde, offset_t addr)
{
   if(!npg_present(pde) || npg_large(pde))
      return 0;

   return __npg_get_pte_nocheck(pde, addr);
}

npg_pte64_t* _npg_get_pte(offset_t addr)
{
   npg_pdpe_t  *pdpe = __npg_get_pdpe(addr);
   npg_pde64_t *pde;

   if(!pdpe || !npg_present(pdpe) || npg_large(pdpe))
      return (npg_pte64_t*)0;

   pde = __npg_get_pde(pdpe, addr);

   if(!pde || !npg_present(pde) || npg_large(pde))
      return (npg_pte64_t*)0;

   return __npg_get_pte(pde, addr);
}



/*
** resolve pdpe (allocate pdp if needed)
*/
static inline npg_pdpe_t* __npg_resolve_pdpe(offset_t addr, uint64_t attr)
{
   vm_pgmem_t  *pg = npg_get_active_paging();
   npg_pml4e_t *pml4e;
   npg_pdpe_t  *pdp;

   pml4e = &pg->pml4[pml4_idx(addr)];
   if(!npg_present(pml4e))
   {
      pdp = __npg_new_pdp();
      /* upper-level entry has full pvl */
      npg_set_entry(pml4e, attr|npg_dft_pvl, page_nr(pdp));
   }
   else
      pdp = (npg_pdpe_t*)page_addr(pml4e->addr);

   return &pdp[pdp_idx(addr)];
}

/*
** resolve pde (allocate pd if needed) for non-large pdpe
** return 0 if large page
*/
static inline
npg_pde64_t* __npg_resolve_pde(npg_pdpe_t *pdpe, offset_t addr, uint64_t attr)
{
   npg_pde64_t *pd;

   if(!npg_present(pdpe))
   {
      pd = __npg_new_pd();
      /* upper-level entry has full pvl */
      npg_set_entry(pdpe, attr|npg_dft_pvl, page_nr(pd));
   }
   else if(npg_large(pdpe))
      return 0;
   else
      pd = (npg_pde64_t*)page_addr(pdpe->addr);

   return &pd[pd64_idx(addr)];
}

/*
** resolve pte (allocate pt if needed for non-large pde
** return 0 if large page
*/
static inline
npg_pte64_t* __npg_resolve_pte(npg_pde64_t *pde, offset_t addr, uint64_t attr)
{
   npg_pte64_t *pt;

   if(!npg_present(pde))
   {
      pt = __npg_new_pt();
      /* upper-level entry has full pvl */
      npg_set_entry(pde, attr|npg_dft_pvl, page_nr(pt));
   }
   else if(npg_large(pde))
      return 0;
   else
      pt = (npg_pte64_t*)page_addr(pde->addr);

   return &pt[pt64_idx(addr)];
}

static void __npg_unmap_2M(npg_pde64_t *pde)
{
   npg_pte64_t  *pt;
   uint32_t     i;

   debug(PG_MAP, "unmap 2M 0x%X\n", pde->raw);
   if(!npg_large(pde))
   {
      pt = (npg_pte64_t*)page_addr(pde->addr);

      debug(PG_MAP, "clear each pte\n");
      for(i=0 ; i<PTE64_PER_PT ; i++)
         if(npg_present(&pt[i]))
            npg_zero(&pt[i]);

      debug(PG_MAP, "freeing pt\n");
      pool_push_page((offset_t)pt);
   }

   npg_zero(pde);
}



/*
** Low level services
*/
static void __npg_map_2M_nolarge(npg_pde64_t *pde, offset_t addr, uint64_t attr)
{
   npg_pte64_t *pt;
   offset_t    pfn;
   uint32_t    i;

   pt  = __npg_new_pt();
   debug(PG_MAP, "new pt\n");
   pfn = pg_4K_nr(addr);
   debug(PG_MAP, "mapping 4K for each pte\n");
   for(i=0 ; i<PTE64_PER_PT ; i++, pfn++)
      npg_set_page_entry(&pt[i], attr, pfn);

   /* upper-level entry has full pvl */
   npg_set_entry(pde, attr|npg_dft_pvl, page_nr(pt));
   debug(PG_MAP, "mapped 2M 0x%X 0x%X\n", addr, attr);
}

static void __npg_map_2M(npg_pde64_t *pde, offset_t addr, uint64_t attr)
{
   if(npg_present(pde))
      __npg_unmap_2M(pde);

   if(info->vm.cpu.skillz.pg_2M)
   {
      npg_set_large_page_entry(pde, attr, pg_2M_nr(addr));
      debug(PG_MAP, "mapped 2M (large) 0x%X 0x%X\n", addr, attr);
      return;
   }

   __npg_map_2M_nolarge(pde, addr, attr);
}

static void __npg_remap_2M_finest(npg_pde64_t *pde, offset_t addr)
{
   uint64_t attr = npg_get_attr(pde);

   if(!pg_2M_aligned(addr))
      addr = pg_2M_align(addr);

   debug(PG_MAP, "remap 2M 0x%X 0x%X\n", addr, attr);
   __npg_unmap_2M(pde);
   __npg_map_2M_nolarge(pde, addr, attr);
}

static void __npg_unmap_1G(npg_pdpe_t *pdpe)
{
   npg_pde64_t *pd;
   uint32_t    i;

   debug(PG_MAP, "unmap 1G 0x%X\n", pdpe->raw);
   if(!npg_large(pdpe))
   {
      pd = (npg_pde64_t*)page_addr(pdpe->addr);

      debug(PG_MAP, "unmap 2M for each pde\n");
      for(i=0 ; i<PDE64_PER_PD ; i++)
         if(npg_present(&pd[i]))
            __npg_unmap_2M(&pd[i]);

      debug(PG_MAP, "freeing pd\n");
      pool_push_page((offset_t)pd);
   }

   npg_zero(pdpe);
}

static void __npg_map_1G_nolarge(npg_pdpe_t *pdpe, offset_t addr, uint64_t attr)
{
   npg_pde64_t *pd;
   uint32_t    i;

   pd = __npg_new_pd();
   debug(PG_MAP, "map 2M for each pde\n");
   for(i=0 ; i<PDE64_PER_PD ; i++)
   {
      __npg_map_2M(&pd[i], addr, attr);
      addr += PG_2M_SIZE;
   }

   /* upper-level entry has full pvl */
   npg_set_entry(pdpe, attr|npg_dft_pvl, page_nr(pd));
   debug(PG_MAP, "mapped 1G until 0x%X 0x%X\n", addr, attr);
}

static void __npg_map_1G(npg_pdpe_t *pdpe, offset_t addr, uint64_t attr)
{
   if(npg_present(pdpe))
      __npg_unmap_1G(pdpe);

   if(info->vm.cpu.skillz.pg_1G)
   {
      npg_set_large_page_entry(pdpe, attr, pg_1G_nr(addr));
      debug(PG_MAP, "mapped 1G (large) 0x%X 0x%X\n", addr, attr);
      return;
   }

   __npg_map_1G_nolarge(pdpe, addr, attr);
}

static void __npg_remap_1G_finest(npg_pdpe_t *pdpe, offset_t addr)
{
   uint64_t attr = npg_get_attr(pdpe);

   if(!pg_1G_aligned(addr))
      addr = pg_1G_align(addr);

   debug(PG_MAP, "remap 1G 0x%X 0x%X\n", addr, attr);
   __npg_unmap_1G(pdpe);
   __npg_map_1G_nolarge(pdpe, addr, attr);
}



/*
** Intermediate services working on ALIGNED addresses
*/
static void _npg_map_512G(offset_t addr, uint64_t attr)
{
   npg_pdpe_t *pdp = __npg_resolve_pdpe(pg_512G_align(addr), attr);
   uint32_t   i;

   debug(PG_MAP, "map 512G 0x%X 0x%X\n", addr, attr);
   for(i=0 ; i<PDPE_PER_PDP ; i++)
   {
      __npg_map_1G(&pdp[i], addr, attr);
      addr += PG_1G_SIZE;
   }
}

static void _npg_map_1G(offset_t addr, uint64_t attr)
{
   npg_pdpe_t *pdpe = __npg_resolve_pdpe(addr, attr);

   debug(PG_MAP, "map 1G 0x%X 0x%X\n", addr, attr);
   __npg_map_1G(pdpe, addr, attr);
}

static void _npg_map_2M(offset_t addr, uint64_t attr)
{
   npg_pdpe_t  *pdpe = __npg_resolve_pdpe(addr, attr);
   npg_pde64_t *pde;

   debug(PG_MAP, "map 2M 0x%X 0x%X\n", addr, attr);
   if(npg_large(pdpe))
      return;

   pde = __npg_resolve_pde(pdpe, addr, attr);
   __npg_map_2M(pde, addr, attr);
}

static void _npg_map_4K(offset_t addr, uint64_t attr)
{
   npg_pdpe_t  *pdpe = __npg_resolve_pdpe(addr, attr);
   npg_pde64_t *pde  = __npg_resolve_pde(pdpe, addr, attr);
   npg_pte64_t *pte;

   debug(PG_MAP, "map 4K 0x%X 0x%X\n", addr, attr);
   if(npg_large(pdpe) || npg_large(pde))
      return;

   pte = __npg_resolve_pte(pde, addr, attr);
   npg_set_page_entry(pte, attr, pg_4K_nr(addr));
}

static void _npg_unmap_512G(offset_t addr, uint64_t __unused__ attr)
{
   npg_pdpe_t *pdp = __npg_get_pdpe(addr);
   uint32_t   i;

   debug(PG_MAP, "unmap 512G 0x%X\n", addr);
   if(!pdp)
      return;

   debug(PG_MAP, "unmap 1G for each pdpe\n");
   for(i=0 ; i<PDPE_PER_PDP ; i++)
      if(npg_present(&pdp[i]))
         __npg_unmap_1G(&pdp[i]);
}

static void _npg_unmap_1G(offset_t addr, uint64_t __unused__ attr)
{
   npg_pdpe_t *pdpe = __npg_get_pdpe(addr);

   debug(PG_MAP, "unmap 1G 0x%X\n", addr);
   if(pdpe && npg_present(pdpe))
      __npg_unmap_1G(pdpe);
}

static void _npg_unmap_2M(offset_t addr, uint64_t __unused__ attr)
{
   npg_pdpe_t  *pdpe = __npg_get_pdpe(addr);
   npg_pde64_t *pde;

   debug(PG_MAP, "unmap 2M 0x%X\n", addr);
   if(!pdpe || !npg_present(pdpe))
   {
      debug(PG_MAP, "unmap 2M pdpe not present\n");
      return;
   }

   if(npg_large(pdpe))
      __npg_remap_1G_finest(pdpe, addr);
   else
      debug(PG_MAP, "unmap 2M pdpe not a large one\n");

   pde = __npg_get_pde(pdpe, addr);
   __npg_unmap_2M(pde);
}

npg_pte64_t* _npg_remap_finest_4K(offset_t addr)
{
   npg_pdpe_t  *pdpe = __npg_get_pdpe(addr);
   npg_pde64_t *pde;

   debug(PG_MAP, "remap 4K finest 0x%X\n", addr);
   if(!pdpe || !npg_present(pdpe))
      return (npg_pte64_t*)0;

   if(npg_large(pdpe))
      __npg_remap_1G_finest(pdpe, addr);

   pde = __npg_get_pde(pdpe, addr);

   if(!pde || !npg_present(pde))
      return (npg_pte64_t*)0;

   if(npg_large(pde))
      __npg_remap_2M_finest(pde, addr);

   return __npg_get_pte(pde, addr);
}

static void _npg_unmap_4K(offset_t addr, uint64_t __unused__ attr)
{
   debug(PG_MAP, "unmap 4K 0x%X\n", addr);
   npg_pte64_t *pte = _npg_remap_finest_4K(addr);

   if(pte)
      npg_zero(pte);
}

static void _npg_remap_512G(offset_t addr, uint64_t attr)
{
   panic("%s(0x%X, 0x%X) not implemented !", __FUNCTION__, addr, attr);
}

static void _npg_remap_1G(offset_t addr, uint64_t attr)
{
   panic("%s(0x%X, 0x%X) not implemented !", __FUNCTION__, addr, attr);
}

static void _npg_remap_2M(offset_t addr, uint64_t attr)
{
   panic("%s(0x%X, 0x%X) not implemented !", __FUNCTION__, addr, attr);
}

static void _npg_remap_4K(offset_t addr, uint64_t attr)
{
   panic("%s(0x%X, 0x%X) not implemented !", __FUNCTION__, addr, attr);
}



/*
** Generic range services
*/
static npg_op_t npg_4K_op =
{
   .sz  = PG_4K_SIZE,
   .shf = PG_4K_SHIFT,
   .fnc = {_npg_map_4K, _npg_unmap_4K, _npg_remap_4K},
   .nxt = (npg_op_t*)0,
};

static npg_op_t npg_2M_op =
{
   .sz  = PG_2M_SIZE,
   .shf = PG_2M_SHIFT,
   .fnc = {_npg_map_2M, _npg_unmap_2M, _npg_remap_2M},
   .nxt = &npg_4K_op,
};

static npg_op_t npg_1G_op =
{
   .sz  = PG_1G_SIZE,
   .shf = PG_1G_SHIFT,
   .fnc = {_npg_map_1G, _npg_unmap_1G, _npg_remap_1G},
   .nxt = &npg_2M_op,
};

static npg_op_t npg_512G_op =
{
   .sz  = PG_512G_SIZE,
   .shf = PG_512G_SHIFT,
   .fnc = {_npg_map_512G, _npg_unmap_512G, _npg_remap_512G},
   .nxt = &npg_1G_op,
};

static inline offset_t __npg_bk_op(offset_t start, offset_t end,
                                   offset_t upper, uint64_t attr,
                                   npg_op_t *op, uint8_t act)
{
   offset_t addr;

   if(op->nxt)
   {
      offset_t start_up = __align_next(start, op->sz);
      bool_t   diff_tbl = (pg_abs_idx(start, op->shf) != pg_abs_idx(end, op->shf));

      if(__aligned(start, op->sz) && diff_tbl)
      {
         op->fnc[act](start, attr);
         addr = start_up;
      }
      else
         addr = __npg_bk_op(start, end, start_up, attr, op->nxt, act);
   }
   else
      addr = __align(start, op->sz);

   while(addr < min(__align(end, op->sz), upper))
   {
      op->fnc[act](addr, attr);
      addr += op->sz;
   }

   return addr;
}

static inline void __npg_fw_op(offset_t start, offset_t end, uint64_t attr,
                               npg_op_t *op, uint8_t act)
{
   offset_t addr = start;

   while(addr < __align(end, op->sz))
   {
      op->fnc[act](addr, attr);
      addr += op->sz;
   }

   if(op->nxt && !__aligned(end, op->sz))
      __npg_fw_op(addr, end, attr, op->nxt, act);
}

static offset_t _npg_bk_op(offset_t start, offset_t end, uint64_t attr, uint8_t act)
{
   debug(PG_MAP, "backward 0x%X 0x%X\n", start, end);
   return __npg_bk_op(start, end, info->vm.cpu.max_paddr, attr, &npg_512G_op, act);
}

static void _npg_fw_op(offset_t start, offset_t end, uint64_t attr, uint8_t act)
{
   debug(PG_MAP, "forward 0x%X 0x%X\n", start, end);
   __npg_fw_op(start, end, attr, &npg_512G_op, act);
}



/*
** High-level services
*/
void npg_map(offset_t start, offset_t end, uint64_t attr)
{
   debug(PG_MAP, "map [0x%X - 0x%X] 0x%X\n", start, end, attr);
   _npg_fw_op(_npg_bk_op(start, end, attr, NPG_OP_MAP), end, attr, NPG_OP_MAP);
}

void npg_unmap(offset_t start, offset_t end)
{
   debug(PG_MAP, "unmap [0x%X - 0x%X]\n", start, end);
   _npg_fw_op(_npg_bk_op(start, end, 0, NPG_OP_UNMAP), end, 0, NPG_OP_UNMAP);
}

void npg_setup_a20()
{
   debug(PG_MAP, "A20 %s (wrap-around not implemented)\n"
         , info->vm.dev.mem.a20?"on":"off");
}
