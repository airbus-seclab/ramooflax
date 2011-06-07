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
#include <vmem.h>
#include <vmm.h>
#include <segmem.h>
#include <realmem.h>
#include <pagemem.h>
#include <paging.h>
#include <string.h>
#include <info_data.h>

extern info_data_t *info;

static void vm_pmode_pagemem_init()
{
   pg_map(0, info->area.start, PG_USR|PG_RW);
   pg_map(info->area.end, info->hrd.mem.top, PG_USR|PG_RW);
}

static void vm_rmode_pagemem_init()
{
   pdpe_t   *pdp;
   pde64_t  *pd;
   pte64_t  *pt;
   uint32_t p;
   size_t   len;

   pdp = info->vm.cpu.pg.rm->pdp;
   pd  = info->vm.cpu.pg.rm->pd;
   pt  = info->vm.cpu.pg.rm->pt;

   __pg_set_entry(&pdp[0] , PG_USR|PG_RW, page_nr(pd));
   __pg_set_entry(&pd[0]  , PG_USR|PG_RW, page_nr(pt));

   for(p=0 ; p<page_nr(RM_LIMIT) ; p++)
      __pg_set_entry(&pt[p], PG_USR|PG_RW, p);

   /* XXX: we are not a 8086 and do not need to emulate wrap-around */
   len = sizeof(pte64_t)*page_nr(RM_WRAP_LIMIT - RM_LIMIT);
   memcpy((void*)&pt[p], (void*)&pt[0], len);
}

static void vm_pagemem_init()
{
   pdpe_t *pdp;

   vm_rmode_pagemem_init();
   vm_pmode_pagemem_init();

   /* default to real mode a20 off */
   pdp = info->vm.cpu.pg.rm->pdp;
   //pdp = info->vm.cpu.pg.pm.pdp[0];
   __pg_set_entry(&info->vm.cpu.pg.rm->pml4[0], PG_USR|PG_RW, page_nr(pdp));
}

/*
** we map more than available
** since we only use 1GB or 2MB pages
*/
static void vmm_pagemem_init()
{
   pml4e_t   *pml4;
   pdpe_t    *pdp;
   pde64_t   *pd;
   cr3_reg_t cr3;
   offset_t  pfn;
   uint32_t  i, j, k;
   offset_t  limit;
   size_t    pdp_nr, pd_nr, pt_nr;
   size_t    pml4e_max, pdpe_max, pde_max;

   pml4   = info->vmm.cpu.pg.pml4;
   limit  = info->hrd.mem.top - 1;
   pdp_nr = pdp_nr(limit) + 1;
   pd_nr  = pd64_nr(limit) + 1;
   pt_nr  = pt64_nr(limit) + 1;
   pfn    = 0;

   pml4e_max = pdp_nr; /* only one pml4 */
   for(i=0 ; i<pml4e_max ; i++)
   {
      pdp = info->vmm.cpu.pg.pdp[i];
      __pg_set_entry(&pml4[i], PG_KRN|PG_RW, page_nr(pdp));

      pdpe_max = min(pd_nr, PDPE_PER_PDP);
      pd_nr -= pdpe_max;

      for(j=0 ; j<pdpe_max ; j++)
      {
	 if(info->vmm.cpu.skillz.pg_1G)
	    __pg_set_large_entry(&pdp[j], PG_KRN|PG_RW, pfn++);
	 else
	 {
	    pd = info->vmm.cpu.pg.pd[j];
	    __pg_set_entry(&pdp[j], PG_KRN|PG_RW, page_nr(pd));

	    pde_max = min(pt_nr, PDE64_PER_PD);
	    pt_nr -= pde_max;

	    for(k=0 ; k<pde_max ; k++)
	       __pg_set_large_entry(&pd[k], PG_KRN|PG_RW, pfn++);
	 }
      }
   }

   cr3.raw = 0UL;
   cr3.pml4.addr = page_nr(pml4);
   set_cr3(cr3.raw);
}

static void pagemem_init()
{
   vmm_pagemem_init();
   vm_pagemem_init();
}

static void segmem_init()
{
   gdt_reg_t        gdtr;
   seg_desc_t       *gdt;
   sys64_seg_desc_t *tss;

   gdt = info->vmm.cpu.sg->gdt;

   gdt[0].raw                = null_desc;
   gdt[vmm_code_seg_idx].raw = code64_desc;
   gdt[vmm_data_seg_idx].raw = data32_desc;

   tss = (sys64_seg_desc_t*)&gdt[vmm_tss_seg_idx];
   tss64_desc(tss, (offset_t)&info->vmm.cpu.sg->tss);
 
   gdtr.desc  = gdt;
   gdtr.limit = sizeof(info->vmm.cpu.sg->gdt) - 1;

   set_gdtr(gdtr);
   set_tr(vmm_tss_sel);
   segmem_reload(vmm_code_sel, vmm_data_sel);
}

void vmem_init()
{
   segmem_init();
   pagemem_init();
}
