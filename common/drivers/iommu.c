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
#include <iommu.h>
#include <acpi.h>
#include <debug.h>
#include <npg.h>
#include <pool.h>
#include <info_data.h>

extern info_data_t *info;

/*
** Debug functions
*/
#ifdef CONFIG_IOMMU_DBG
static void __dmar_drhd_fault_status(dmar_flt_sts_reg_t sts)
{
   debug(IOMMU,
         "dmar drhd fault status register: 0x%x\n"
         "pfo %d ppf %d afo %d apf %d iqe %d ice %d ite %d pro %d fri %d\n"
         ,sts.raw, sts.pfo, sts.ppf, sts.afo, sts.apf
         ,sts.iqe, sts.ice, sts.ite, sts.pro, sts.fri);
}

#ifdef __INIT__
static void __dmar_drhd_cap(dmar_cap_reg_t cap)
{
   debug(IOMMU,
         "dmar drhd capabilities: 0x%X\n"
         " nd    %d  afl  %d  rwbf %d  plmr %d    phmr  %d  cm  %d\n"
         " sagaw %d  mgaw %d  zlr  %d  fro  0x%x  sllps %d  psi %d\n"
         " nfr   %d  mamv %d  dwd  %d  drd  %d    fl1gp %d  pi  %d\n"
         ,cap.raw,cap.nd,cap.afl,cap.rwbf,cap.plmr,cap.phmr,cap.cm
         ,cap.sagaw,cap.mgaw,cap.zlr,cap.fro,cap.sllps,cap.psi
         ,cap.nfr,cap.mamv,cap.dwd,cap.drd,cap.fl1gp,cap.pi);
}

static void __dmar_drhd_xcap(dmar_xcap_reg_t xcap)
{
   debug(IOMMU,
         "dmar drhd extended capabilities: 0x%X\n"
         " c    %d  qi   %d  dt   %d  ir   %d  eim   %d  pt  %d  sc  %d iro 0x%x\n"
         " mhmv %d  ecs  %d  mts  %d  nest %d  dis   %d  prs %d  ers %d\n"
         " srs  %d  nwfs %d  eafs %d  pss  %d  pasid %d  dit %d  pds %d\n"
         ,xcap.raw,xcap.c,xcap.qi,xcap.dt,xcap.ir,xcap.eim,xcap.pt,xcap.sc,xcap.iro
         ,xcap.mhmv,xcap.ecs,xcap.mts,xcap.nest,xcap.dis,xcap.prs,xcap.ers
         ,xcap.srs,xcap.nwfs,xcap.eafs,xcap.pss,xcap.pasid,xcap.dit,xcap.pds);
}

/* static void dmar_drhd_fault_status(dmar_drhd_info_t *drhd) */
/* { */
/*    dmar_flt_sts_reg_t sts = {.raw = drhd->reg->fault_sts.raw}; */
/*    dmar_flt_evc_reg_t evc = {.raw = drhd->reg->fault_evc.raw}; */

/*    __dmar_drhd_fault_status(sts); */
/*    debug(IOMMU, "dmar drhd fault control register: ip %d\n", evc.ip); */
/* } */

/* static void dmar_drhd_status(dmar_drhd_info_t *drhd) */
/* { */
/*    dmar_cmd_sts_reg_t sts = {.raw = drhd->reg->sts.raw}; */

/*    debug(IOMMU, */
/*          "dmar drhd global status register: 0x%x\n" */
/*          "cfi %d irtp %d ire %d qie %d wbf %d eafl %d fl %d rtp %d te %d\n" */
/*          ,sts.raw, sts.cfi,  sts.irtp, sts.ire, sts.qie */
/*          ,sts.wbf, sts.eafl, sts.fl,   sts.rtp, sts.te); */
/* } */
#endif

static int dmar_drhd_pg_walk(dmar_drhd_info_t *drhd, uint16_t sid, uint64_t addr)
{
   uint8_t bus = sid>>8;
   uint8_t dev = (sid>>3) & 0x1f;
   uint8_t fnc = sid & 7;

   dmar_roote_t *roote;
   dmar_ctxe_t  *ctxe;
   sl_pml4e_t   *pml4e;
   sl_pdpe_t    *pdp, *pdpe;
   sl_pde64_t   *pd, *pde;
   sl_pte64_t   *pt, *pte;

   roote = &drhd->root[bus];
   debug(IOMMU, "dmar root entry 0x%X\n", roote->raw);

   if(!roote->p)
   {
      debug(IOMMU, "dmar root entry not present\n");
      return VM_FAULT;
   }

   ctxe = &drhd->ctx[dev*8 + fnc];
   debug(IOMMU, "dmar ctx entry 0x%X\n", ctxe->low.raw);

   if(!ctxe->p)
   {
      debug(IOMMU, "dmar ctx entry not present\n");
      return VM_FAULT;
   }

   pml4e = &drhd->slt[pml4_idx(addr)];
   debug(IOMMU, "dmar slt pml4e 0x%X\n", pml4e->raw);

   if(!slt_pg_present(pml4e))
   {
      debug(IOMMU, "dmar slt pml4e not present\n");
      return VM_FAULT;
   }

   pdp  = (sl_pdpe_t*)page_addr(pml4e->addr);
   pdpe = &pdp[pdp_idx(addr)];
   debug(IOMMU, "dmar slt pdpe 0x%X\n", pdpe->raw);

   if(!slt_pg_present(pdpe))
   {
      debug(IOMMU, "dmar slt pdpe not present\n");
      return VM_FAULT;
   }

   pd  = (sl_pde64_t*)page_addr(pdpe->addr);
   pde = &pd[pd64_idx(addr)];
   debug(IOMMU, "dmar slt pde 0x%X\n", pde->raw);

   if(!slt_pg_present(pde))
   {
      debug(IOMMU, "dmar slt pde not present\n");
      return VM_FAULT;
   }

   pt  = (sl_pte64_t*)page_addr(pde->addr);
   pte = &pt[pt64_idx(addr)];
   debug(IOMMU, "dmar slt pte 0x%X\n", pte->raw);

   if(!slt_pg_present(pte))
   {
      debug(IOMMU, "dmar slt pte not present\n");
      return VM_FAULT;
   }

   return VM_DONE;
}
#endif

/*
** DMAR drhd fault status checking
*/
void dmar_drhd_fault_check(dmar_drhd_info_t *drhd)
{
   volatile dmar_flt_rec_reg_t  *frr_ptr;
   dmar_flt_rec_reg_t            frr;
   dmar_flt_sts_reg_t            sts;
   dmar_cap_reg_t                cap;
   loc_t                         loc;

   sts.raw = drhd->reg->fault_sts.raw;

   if(!sts.ppf)
      return ;

   cap.raw    = drhd->cap.raw;
   loc.linear = (offset_t)drhd->reg + 16 * cap.fro;
   frr_ptr    = (volatile dmar_flt_rec_reg_t*)loc.addr;
   frr.high   = frr_ptr[sts.fri].high;

#ifdef CONFIG_IOMMU_DBG
   frr.low    = frr_ptr[sts.fri].low;

   __dmar_drhd_fault_status(sts);

   debug(IOMMU, "fault[%d]: f %d t %d fr %d sid 0x%x (%x:%x:%x) fi 0x%X\n"
         ,sts.fri, frr.f, frr.t, frr.fr
         ,frr.sid, frr.sid>>8, (frr.sid>>3)&0x1f, frr.sid&7
         ,frr.low);

   dmar_drhd_pg_walk(drhd, frr.sid, frr.low.raw);
#endif

   /* clear fault */
   frr_ptr[sts.fri].high = frr.high;

   /* clear overflow */
   if(sts.pfo)
      drhd->reg->fault_sts.raw = sts.raw;

   panic("dmar drhd iommu fault");
}

#ifdef __INIT__
/*
** DMAR global command updater
*/
static int dmar_drhd_global_cmd_update(dmar_drhd_info_t *drhd, uint32_t bit)
{
   dmar_cmd_sts_reg_t cmd;

   cmd.raw = (drhd->reg->sts.raw & 0x96ffffff) | bit;
   drhd->reg->cmd.raw = cmd.raw;

   /* XXX: retry threshold */
   debug(IOMMU, "updating drhd sts cmd register\n");
   do io_wait(1000);
   while((drhd->reg->sts.raw & bit) != bit);

   return VM_DONE;
}


/*
** DMAR root table
*/
static int dmar_drhd_root_init(dmar_drhd_info_t *drhd)
{
   size_t i;

   drhd->root = (dmar_roote_t*)pool_pop_page();
   if(!drhd->root)
   {
      debug(IOMMU, "no more page for root table\n");
      return VM_FAIL;
   }

   /* memset((void*)drhd->root, 0, PAGE_SIZE); */
   for(i=0 ; i<256 ; i++)
      drhd->root[i].raw = (offset_t)drhd->ctx | 1;

   /*
   ** Flush caches for chipset
   ** vmm memory is covered by WB mtrr
   */
   wbinvd();

   /* setup root table register */
   drhd->reg->root_addr.raw = (offset_t)drhd->root;
   return VM_DONE;
}

/*
** DMAR context table
*/
static int dmar_drhd_ctx_init(dmar_drhd_info_t *drhd)
{
   size_t i;

   drhd->ctx = (dmar_ctxe_t*)pool_pop_page();
   if(!drhd->ctx)
   {
      debug(IOMMU, "no more page for context table\n");
      return VM_FAIL;
   }

   /*
   ** Contexte table:
   ** - device id: 1
   ** - transalation type: Untranslated
   ** - max agaw
   */
   /* memset((void*)drhd->ctx, 0, PAGE_SIZE); */
   for(i=0 ; i<256 ; i++)
   {
      drhd->ctx[i].did     = 1;
      drhd->ctx[i].aw      = drhd->agaw;
      drhd->ctx[i].low.raw = (offset_t)drhd->slt | 1;
   }

   return VM_DONE;
}

/*
** DMAR Second Level page table
*/
static int dmar_drhd_slt_init(dmar_drhd_info_t *drhd)
{
   size_t       j,k,l;
   sl_pdpe_t   *pdp;
   sl_pde64_t  *pd;
   sl_pte64_t  *pt;

   drhd->slt = (sl_pml4e_t*)pool_pop_page();
   if(!drhd->slt)
   {
      debug(IOMMU, "no more page for SLT\n");
      return VM_FAIL;
   }

   pdp = (sl_pdpe_t*)pool_pop_page();
   if(!pdp)
   {
      debug(IOMMU, "no more page for SLT pdp\n");
      return VM_FAIL;
   }

   memset((void*)drhd->slt, 0, PAGE_SIZE);
   memset((void*)pdp, 0, PAGE_SIZE);
   drhd->slt[0].raw = (offset_t)pdp | 7;

   /* XXX: map 9 GB, we should refer to smap */
   for(j=0 ; j<9 ; j++)
   {
      pd = (sl_pde64_t*)pool_pop_page();
      if(!pd)
      {
         debug(IOMMU, "no more page for pd %d\n", j);
         return VM_FAIL;
      }

      /* memset((void*)pd, 0, PAGE_SIZE); */
      pdp[j].raw = (offset_t)pd | 7;

      for(k=0 ; k<PDE64_PER_PD ; k++)
      {
         pt = (sl_pte64_t*)pool_pop_page();
         if(!pt)
         {
            debug(IOMMU, "no more page for pt %d\n", k);
            return VM_FAIL;
         }

         /* memset((void*)pt, 0, PAGE_SIZE); */
         pd[k].raw = (offset_t)pt | 7;

         for(l=0 ; l<PTE64_PER_PT ; l++)
         {
            offset_t addr = pg_1G_addr(j) | pg_2M_addr(k) | pg_4K_addr(l);
            offset_t off  = 0;

            /* protect vmm area */
            if(addr >= info->area.start && addr < info->area.end)
               continue;

            pt[l].raw = (addr+off) | 7;
         }
      }
   }

   return VM_DONE;
}

/*
** DMAR fault handling
**
** XXX: trigger MSI, however:
** - we should discover IO APIC ID first
** - we do not intercept interrupts for now
**
** Not implemented, we rather inspect (lame)
** fault status register during each VMEXIT
*/
static int dmar_drhd_fault_init(dmar_drhd_info_t *drhd)
{
   /* dmar_flt_eva_reg_t eva; */
   /* eva.raw = 0; */
   /* eva.did = 2; /\* I/O APIC ID *\/ */
   /* drhd->reg->fault_eva.raw = eva.raw; */

   /* drhd->reg->fault_evd.raw = 0xc0; /\* vector 0xc0, dm=fixed (0) *\/ */

   drhd->reg->fault_evc.raw = 0;
   return VM_DONE;
}

/*
** Check capabilities
*/
static int dmar_drhd_cap_init(dmar_drhd_info_t *drhd)
{
   drhd->cap.raw = drhd->reg->cap.raw;

#ifdef CONFIG_IOMMU_DBG
   __dmar_drhd_cap(drhd->cap);
   drhd->xcap.raw= drhd->reg->xcap.raw;
   __dmar_drhd_xcap(drhd->xcap);
#endif

   if(drhd->cap.sagaw & DMAR_DRHD_CAP_AGAW_48)
      drhd->agaw = DMAR_DRHD_CTX_AW_48;
   else if(drhd->cap.sagaw & DMAR_DRHD_CAP_AGAW_39)
      drhd->agaw = DMAR_DRHD_CTX_AW_39;
   else
   {
      debug(IOMMU, "context table: unsupported AGAW");
      return VM_FAIL;
   }

   debug(IOMMU, "dmar drhd contex table AGAW %d\n", drhd->agaw);
   return VM_DONE;
}

/*
** DMAR MMIO Registers Nested Paging unmapping
*/
static int dmar_drhd_protect(dmar_drhd_info_t *drhd)
{
   offset_t start = (offset_t)drhd->reg;
   offset_t end   = start + PAGE_SIZE;

   npg_unmap(start, end);
   debug(IOMMU, "protect DRHD mmio space [0x%X - 0x%X]\n", start, end);
   return VM_DONE;
}

/*
** Init Intel VT-d DMAR DRHD engine
*/
static int dmar_drhd_setup(dmar_drhd_info_t *drhd)
{
   if(dmar_drhd_protect(drhd) != VM_DONE)
      return VM_FAIL;

   if(dmar_drhd_cap_init(drhd) != VM_DONE)
      return VM_FAIL;

   if(dmar_drhd_fault_init(drhd) != VM_DONE)
      return VM_FAIL;

   if(dmar_drhd_slt_init(drhd) != VM_DONE)
      return VM_FAIL;

   if(dmar_drhd_ctx_init(drhd) != VM_DONE)
      return VM_FAIL;

   if(dmar_drhd_root_init(drhd) != VM_DONE)
      return VM_FAIL;

   return VM_DONE;
}

static int dmar_drhd_init(dmar_drhd_info_t *drhd)
{
   dmar_cmd_sts_reg_t cmd;

   if(dmar_drhd_setup(drhd) != VM_DONE)
      return VM_FAIL;

   /* enable dma remapping engine */
   cmd.raw = 0;
   cmd.rtp = 1;
   dmar_drhd_global_cmd_update(drhd, cmd.raw);
   debug(IOMMU, "enabled root table pointer\n");

   cmd.raw = 0;
   cmd.te = 1;
   dmar_drhd_global_cmd_update(drhd, cmd.raw);
   debug(IOMMU, "enabled DMA remapping engine\n");

   dmar_drhd_fault_check(drhd);
   return VM_DONE;
}

/*
** We only init DRHD covering all (not defined in other DRHDs)
** remaining cpi devices.
**
** ex: on Chipset Q87, we have two DRHD, one for VGA controller and
** another one for remaining pci devices
*/
void iommu_init()
{
   dmar_gen_t  *gen;
   loc_t        struc;
   size_t       n;

   debug(IOMMU, "\n- iommu init\nacpi dmar 0x%X\n", info->hrd.acpi.dmar);

   struc.addr = &info->hrd.acpi.dmar->structures;
   n = info->hrd.acpi.dmar->length - sizeof(acpi_dmar_t);

   while(n)
   {
      gen = (dmar_gen_t*)struc.addr;

      if(gen->type == DMAR_STRUCT_DRHD)
      {
         dmar_drhd_t *drhd = (dmar_drhd_t*)gen;

         debug(IOMMU, "dmar drhd @ 0x%X reg @ 0x%X\n", drhd, drhd->bar);

         /* only one per system */
         if(drhd->flags.include_pci_all)
         {
            info->hrd.iommu.all.reg = (volatile dmar_drhd_reg_t*)drhd->bar;

            if(dmar_drhd_init(&info->hrd.iommu.all) != VM_DONE)
               panic("could not init DMAR DRHD 0x%X\n", drhd->bar);
         }
      }
      else if(gen->type == DMAR_STRUCT_RMRR)
         warning(IOMMU, "dmar unhandled structure RMRR\n");
      else if(gen->type == DMAR_STRUCT_ATSR)
         warning(IOMMU, "dmar unhandled structure ATSR\n");
      else if(gen->type == DMAR_STRUCT_RHSA)
         warning(IOMMU, "dmar unhandled structure RHSA\n");
      else if(gen->type == DMAR_STRUCT_ANDD)
         warning(IOMMU, "dmar unhandled structure ANDD\n");

      n            -= gen->length;
      struc.linear += gen->length;
   }
}
#endif
