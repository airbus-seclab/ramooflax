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
#include <acpi.h>
#include <cr.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

#ifdef __INIT__
/*
** 16B boundary in [0xe0000 - 0xfffff]
*/
static int acpi_rsdp_get(offset_t *addr)
{
   loc_t rsdp = {.linear = 0xe0000};

   while(rsdp.linear < 0x100000)
   {
      if(*rsdp.u64 == ACPI_RSDP)
      {
         debug(ACPI, "acpi rsdp 0x%X\n", rsdp.linear);
         *addr = rsdp.linear;
         return VM_DONE;
      }

      rsdp.linear += 16;
   }

   debug(ACPI, "can't find acpi rsd ptr\n");
   return VM_FAIL;
}

static int acpi_fadt_parse(acpi_info_t *acpi)
{
   acpi_fadt_t *fadt = acpi->fadt;

   if(fadt->flags.hw_reduced)
   {
      debug(ACPI, "acpi hardware reduced mode\n");
      return VM_FAIL;
   }

   if(fadt->facs)
      acpi->facs = (acpi_facs_t*)((offset_t)fadt->facs);
   else if(fadt->x_facs)
      acpi->facs = (acpi_facs_t*)((offset_t)fadt->x_facs);
   else
   {
      debug(ACPI, "can't find acpi facs\n");
      return VM_FAIL;
   }

   if(!fadt->pm1a_cnt_blk)
   {
      debug(ACPI, "acpi unsupported fadt pm1a ctl blk\n");
      return VM_FAIL;
   }

   acpi->pm1_ctl_port = fadt->pm1a_cnt_blk;
   debug(ACPI, "acpi pm1a control port 0x%x\n", acpi->pm1_ctl_port);
   return VM_DONE;
}

static int acpi_xsdt_parse(acpi_info_t *acpi)
{
   acpi_sys_hdr_t *sys;
   size_t         nr, i;
   char           *err;

   acpi->xsdt = (acpi_xsdt_t*)acpi->rsdp->xsdt;
   nr = (acpi->xsdt->length - sizeof(acpi_sys_hdr_t))/sizeof(uint64_t);

   debug(ACPI, "acpi xsdt 0x%X (%d entries)\n", acpi->xsdt, nr);
   for(i=0 ; i<nr ; i++)
   {
      sys = (acpi_sys_hdr_t*)acpi->xsdt->entry[i];
      if(sys->signature == ACPI_FADT)
      {
         acpi->fadt = (acpi_fadt_t*)sys;
         if(acpi_fadt_parse(acpi) != VM_DONE)
            return VM_FAIL;
      }
#ifdef CONFIG_IOMMU
      else if(sys->signature == ACPI_DMAR)
         acpi->dmar = (acpi_dmar_t*)sys;
#endif
   }

   if(!acpi->fadt)
   {
      err = "fadt";
      goto __fail;
   }

#ifdef CONFIG_IOMMU
   if(!acpi->dmar)
   {
      err = "dmar";
      goto __fail;
   }
#endif

   return VM_DONE;

__fail:
   debug(ACPI, "acpi xsdt missing %s\n", err);
   return VM_FAIL;
}

static int acpi_opt_hdl(char *str, void *arg)
{
   uint64_t data;

   if(!dec_to_uint64((uint8_t*)str, strlen(str), &data))
      return 0;

   *(uint8_t*)arg = (uint8_t)data;
   return 1;
}

static int acpi_params(acpi_info_t *acpi, mbi_t *mbi)
{
   module_t      *mod = (module_t*)((offset_t)mbi->mods_addr + sizeof(module_t));
   mbi_opt_hdl_t  hdl = (mbi_opt_hdl_t)acpi_opt_hdl;

   if(!mbi_get_opt(mbi, mod, "s3", hdl, (void*)&acpi->s3))
   {
      debug(ACPI, "acpi need s3=x on vmm module cmd line\n");
      return VM_FAIL;
   }

   return VM_DONE;
}

void acpi_init(mbi_t *mbi)
{
   acpi_info_t *acpi = &info->hrd.acpi;

   debug(ACPI, "\n- acpi init\n");

   if(acpi_rsdp_get((offset_t*)&acpi->rsdp) != VM_DONE)
      goto __acpi_panic;

   if(!acpi->rsdp->xsdt)
   {
      debug(ACPI, "no XSDT available\n");
      goto __acpi_panic;
   }

   if(acpi_xsdt_parse(acpi) != VM_DONE)
      goto __acpi_panic;

   if(acpi_params(acpi, mbi) != VM_DONE)
      goto __acpi_panic;

   /* success */
   return;

__acpi_panic:
   panic("acpi init");
}

#else

static int acpi_facs_wake_vector(acpi_info_t *acpi, offset_t *vector)
{
   acpi_facs_t *facs = acpi->facs;

   /* 32/64bit mode wake vector */
   if(facs->x_waking_vector != 0)
   {
      debug(ACPI, "acpi unsupported wake vector CPU mode (32/64)\n");
      return VM_FAIL;
   }

   if(!facs->waking_vector)
   {
      debug(ACPI, "acpi empty wake vector\n");
      return VM_FAIL;
   }

   *vector = facs->waking_vector;
   return VM_DONE;
}

/*
** Emulate ACPI wake up event
** - resume OSPM from FACS wake vector
** - we only support real mode wake up
*/
int acpi_wake_up()
{
   offset_t  vector;
   cr0_reg_t cr0 = {.raw = 0x30};

   if(acpi_facs_wake_vector(&info->hrd.acpi, &vector) != VM_DONE)
      return VM_FAIL;

   __resolve_cr0_wr(&cr0);

   vm_state.cs.limit.raw = 0xffff;
   vmcs_dirty(vm_state.cs.limit);

   vm_state.cs.attributes.raw = VMX_CODE_16_R0_CO_ATTR;
   vmcs_dirty(vm_state.cs.attributes);

   vm_state.rflags.r1 = 1;
   vm_state.rflags.IF = 0;
   vmcs_dirty(vm_state.rflags);

   vm_state.rip.raw = vector & 0xf;
   vmcs_dirty(vm_state.rip);

   vm_state.cs.selector.raw = vector>>4;
   vmcs_dirty(vm_state.cs.selector);

   vm_state.cs.base.raw = vm_state.cs.selector.raw<<4;
   vmcs_dirty(vm_state.cs.base);

   debug(VM, "acpi wake up\n");
   return VM_DONE_LET_RIP;
}
#endif
