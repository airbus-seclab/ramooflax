/*
** Copyright (C) 2014 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#include <pci.h>
#include <pci_e1000.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

static int __dev_pci_dvd_filter(void *data, void __unused__ *arg)
{
   pci_cfg_addr_t    *net  = &info->hrd.dev.net.pci.addr;
   pci_cfg_addr_t    *addr = &info->vm.dev.pci_addr;
   pci_cfg_dev_vdr_t *dvd  = (pci_cfg_dev_vdr_t*)data;

   /* debug(DEV_PCI, "pci io dvd filter: 0x%x\n", dvd->raw); */

   if(addr->bus != net->bus ||
      addr->fnc != net->fnc ||
      addr->dev != net->dev ||
      addr->reg != PCI_CFG_DEV_VDR_OFFSET)
      return VM_DONE;

   if(dvd->vendor != PCI_CFG_VENDOR_INTEL)
      return VM_DONE;

   if(dvd->device != PCI_CFG_DEVICE_i82540EM &&
      dvd->device != PCI_CFG_DEVICE_i82545EM_C)
      return VM_DONE;

   debug(DEV_PCI, "pci io filter: ninja netcard dev\n");
   dvd->device = PCI_CFG_DEVICE_INVALID;
   return VM_DONE;
}

static int __dev_pci_cmd_sts_filter(void *data, void __unused__ *arg)
{
   pci_cfg_addr_t    *net  = &info->hrd.dev.net.pci.addr;
   pci_cfg_addr_t    *addr = &info->vm.dev.pci_addr;
   pci_cfg_cmd_sts_t *cs   = (pci_cfg_cmd_sts_t*)data;

   /* debug(DEV_PCI, "pci io cmd/sts filter: 0x%x\n", cs->raw); */

   if(addr->bus != net->bus ||
      addr->fnc != net->fnc ||
      addr->dev != net->dev ||
      addr->reg != PCI_CFG_CMD_STS_OFFSET)
      return VM_DONE;

   if(!cs->cmd.mm || !cs->cmd.bus_master)
   {
      debug(DEV_PCI, "pci io filter: ninja netcard mm/dma\n");
      cs->cmd.mm = 1;
      cs->cmd.bus_master = 1;
   }

   return VM_DONE;
}

/* static void __dev_pci_dflt_data_filter(void *data) */
/* { */
/*    debug(DEV_PCI, "pci io filter: netcard data 0x%x\n", ((raw32_t*)data)->raw); */
/*    return VM_DONE; */
/* } */

static int __dev_pci_addr_filter(void *data, void __unused__ *arg)
{
   pci_cfg_addr_t *net  = &info->hrd.dev.net.pci.addr;
   pci_cfg_addr_t *addr = &info->vm.dev.pci_addr;

   addr->raw = ((pci_cfg_addr_t*)data)->raw;

   /* debug(DEV_PCI, "pci io addr filter: b%d f%d d%d r%d\n" */
   /* 	 ,addr->bus, addr->fnc, addr->dev, addr->reg); */

   if(addr->bus != net->bus || addr->fnc != net->fnc || addr->dev != net->dev)
      goto __release_data;

   switch(addr->reg)
   {
   case PCI_CFG_DEV_VDR_OFFSET:
      debug(DEV_PCI, "pci io filter: netcard dvd\n");
      info->vm.dev.pci_filter = __dev_pci_dvd_filter;
      goto __filter_data;
   case PCI_CFG_CMD_STS_OFFSET:
      debug(DEV_PCI, "pci io filter: netcard cmd/sts\n");
      info->vm.dev.pci_filter = __dev_pci_cmd_sts_filter;
      goto __filter_data;

   default:
      goto __release_data;
      /* debug(DEV_PCI, "pci io filter: netcard pci reg 0x%x\n", addr->reg); */
      /* info->vm.dev.pci_filter = __dev_pci_dflt_data_filter; */
   }

__filter_data:
   __deny_io(PCI_CONFIG_DATA);
   return VM_DONE;

__release_data:
   __allow_io(PCI_CONFIG_DATA);
   return VM_DONE;
}

int dev_pci(io_insn_t *io)
{
   switch(io->port)
   {
   case PCI_CONFIG_ADDR: info->vm.dev.pci_filter = __dev_pci_addr_filter; break;
   case PCI_CONFIG_DATA: /* set by pci addr filter */ break;
   default             : info->vm.dev.pci_filter = NULL;
   }

   return dev_io_proxify_filter(io, info->vm.dev.pci_filter, NULL);
}
