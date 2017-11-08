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
#include <init.h>
#include <elf.h>
#include <print.h>
#include <segmem.h>
#include <pagemem.h>
#include <cpuid.h>
#include <msr.h>
#include <cr.h>

static pml4e_t    __attribute__((aligned(PAGE_SIZE)))  pml4[PML4E_PER_PML4];
static pdpe_t     __attribute__((aligned(PAGE_SIZE)))  pdp[PDPE_PER_PDP];
static pde64_t    __attribute__((aligned(PAGE_SIZE)))  pd[4][PDE64_PER_PD];
static seg_desc_t __attribute__((aligned(8)))          gdt[] =
{
   { { .raw = null_desc   } },
   { { .raw = code32_desc } },
   { { .raw = code64_desc } },
   { { .raw = data32_desc } },
};

static void init_segmem()
{
   gdt_reg_t gdtr;

   gdtr.limit = sizeof(gdt) - 1;
   gdtr.desc  = &gdt[0];

   set_gdtr(gdtr);
   segmem_reload(gdt_krn_seg_sel(1), gdt_krn_seg_sel(3));
}

static void init_pagemem_1G()
{
   uint32_t p;

   for(p=0 ; p<4 ; p++)
      pg_set_large_entry(&pdp[p], PG_KRN|PG_RW, p);
}

static void init_pagemem_2M()
{
   uint32_t base, p, n;

   for(p=0 ; p<4 ; p++)
   {
      base = (p<<PG_1G_SHIFT)>>PG_2M_SHIFT;
      pg_set_entry(&pdp[p], PG_KRN|PG_RW, page_nr(pd[p]));

      for(n=0 ; n<PDE64_PER_PD ; n++)
         pg_set_large_entry(&pd[p][n], PG_KRN|PG_RW, base+n);
   }
}

static void init_pagemem()
{
   cr3_reg_t cr3;

   pg_set_entry(&pml4[0], PG_KRN|PG_RW, page_nr(pdp));

   if(page_1G_supported())
      init_pagemem_1G();
   else
      init_pagemem_2M();

   cr3.raw = 0ULL;
   cr3.pml4.addr = page_nr(pml4);

   set_cr4(get_cr4()|CR4_PAE|CR4_PSE);
   set_cr3(cr3.low);
   lm_enable();
}

static module_t* setup_module(mbi_t *mbi)
{
   uint32_t count = mbi->mods_count;
   module_t *mod  = (module_t*)mbi->mods_addr;
   offset_t mods_end = 0;

   while(count--) mods_end = ((module_t*)mbi->mods_addr)->mod_end;

   elf_module_load(mod, mods_end);
   return mod;
}

static void enter_lmode(mbi_t *mbi)
{
   fptr32_t entry;
   module_t *mod = setup_module(mbi);

   entry.segment = gdt_krn_seg_sel(2);
   entry.offset  = (uint32_t)elf_module_entry(mod);

   set_cr0(CR0_PG|CR0_ET|CR0_PE);
   set_edi(mbi);
   farjump(entry);
}

static void validate(mbi_t *mbi)
{
   if(mbi->mods_count < 1)
      panic("invalid module count");

   mbi_check_boot_loader(mbi);
   check_cpu_skillz();
}

void __regparm__(1) init(mbi_t *mbi)
{
   validate(mbi);

   init_segmem();
   init_pagemem();

   enter_lmode(mbi);
}
