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
#include <pci.h>
#include <pci_e1000.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

/* static void __dev_pci_dflt_data_filter(void *data) */
/* { */
/*    debug(DEV_PCI, "pci io filter: netcard data 0x%x\n", ((raw32_t*)data)->raw); */
/*    return VM_DONE; */
/* } */

#if defined(CONFIG_HAS_NET) && defined(CONFIG_HAS_E1000)
static int __dev_pci_filter_dvd_e1000(void *data, void __unused__ *arg)
{
   /* pci_cfg_addr_t    *net  = &info->hrd.dev.net.pci.addr; */
   /* pci_cfg_addr_t    *addr = &info->vm.dev.pci_addr; */
   pci_cfg_dev_vdr_t *dvd  = (pci_cfg_dev_vdr_t*)data;

   /* debug(DEV_PCI, "pci io dvd filter: 0x%x\n", dvd->raw); */

   /* if(addr->bus != net->bus || */
   /*    addr->fnc != net->fnc || */
   /*    addr->dev != net->dev || */
   /*    addr->reg != PCI_CFG_DEV_VDR_OFFSET) */
   /*    return VM_DONE; */

   if(dvd->vendor != PCI_CFG_VENDOR_INTEL)
      return VM_DONE;

   if(dvd->device != PCI_CFG_DEVICE_i82540EM &&
      dvd->device != PCI_CFG_DEVICE_i82545EM_C)
      return VM_DONE;

   debug(DEV_PCI, "pci io filter: ninja netcard dev\n");
   dvd->device = PCI_CFG_DEVICE_INVALID;
   return VM_DONE;
}

static int __dev_pci_filter_cmd_sts_e1000(void *data, void __unused__ *arg)
{
   /* pci_cfg_addr_t    *net  = &info->hrd.dev.net.pci.addr; */
   /* pci_cfg_addr_t    *addr = &info->vm.dev.pci_addr; */
   pci_cfg_cmd_sts_t *cs   = (pci_cfg_cmd_sts_t*)data;

   /* debug(DEV_PCI, "pci io cmd/sts filter: 0x%x\n", cs->raw); */

   /* if(addr->bus != net->bus || */
   /*    addr->fnc != net->fnc || */
   /*    addr->dev != net->dev || */
   /*    addr->reg != PCI_CFG_CMD_STS_OFFSET) */
   /*    return VM_DONE; */

   if(!cs->cmd.mm || !cs->cmd.bus_master)
   {
      debug(DEV_PCI, "pci io filter: ninja netcard mm/dma\n");
      cs->cmd.mm = 1;
      cs->cmd.bus_master = 1;
   }

   return VM_DONE;
}

static int __dev_pci_filter_e1000(pci_cfg_addr_t *net, pci_cfg_addr_t *addr)
{
   if(addr->bus != net->bus || addr->fnc != net->fnc || addr->dev != net->dev)
      return VM_IGNORE;

   if(addr->reg == PCI_CFG_DEV_VDR_OFFSET)
   {
      debug(DEV_PCI, "pci io filter: netcard dvd\n");
      info->vm.dev.pci_filter = __dev_pci_filter_dvd_e1000;
      return VM_DONE;
   }

   if(addr->reg == PCI_CFG_CMD_STS_OFFSET)
   {
      debug(DEV_PCI, "pci io filter: netcard cmd/sts\n");
      info->vm.dev.pci_filter = __dev_pci_filter_cmd_sts_e1000;
      return VM_DONE;
   }

   /* debug(DEV_PCI, "pci io filter: netcard pci reg 0x%x\n", addr->reg); */
   /* info->vm.dev.pci_filter = __dev_pci_dflt_data_filter; */
   /* return VM_DONE; */

   return VM_IGNORE;
}
#endif

#if defined(CONFIG_HAS_EHCI) && defined(CONFIG_EHCI_2ND)
static int __dev_pci_filter_dvd_ehci(void *data, void __unused__ *arg)
{
   pci_cfg_dev_vdr_t *ehci = &info->hrd.dev.dbgp.dvd;
   pci_cfg_dev_vdr_t *dvd  = (pci_cfg_dev_vdr_t*)data;

   /* debug(DEV_PCI, "pci io dvd filter: 0x%x\n", dvd->raw); */

   /* if(addr->bus != ehci->bus || */
   /*    addr->fnc != ehci->fnc || */
   /*    addr->dev != ehci->dev || */
   /*    addr->reg != PCI_CFG_DEV_VDR_OFFSET) */
   /*    return VM_DONE; */

   if(dvd->raw != ehci->raw)
      return VM_DONE;

   debug(DEV_PCI, "pci io filter: ninja ehci dev: %d:%d:%d\n"
         ,info->hrd.dev.dbgp.pci.addr.bus
         ,info->hrd.dev.dbgp.pci.addr.fnc
         ,info->hrd.dev.dbgp.pci.addr.dev);

   dvd->device = PCI_CFG_DEVICE_INVALID;
   return VM_DONE;
}

static int __dev_pci_filter_ehci(pci_cfg_addr_t *ehci, pci_cfg_addr_t *addr)
{
   if(addr->bus == ehci->bus &&
      addr->fnc == ehci->fnc &&
      addr->dev == ehci->dev &&
      addr->reg == PCI_CFG_DEV_VDR_OFFSET)
   {
      debug(DEV_PCI, "pci io filter: ehci dvd\n");
      info->vm.dev.pci_filter = __dev_pci_filter_dvd_ehci;
      return VM_DONE;
   }

   return VM_IGNORE;
}
#endif

static int __dev_pci_filter_addr(void *data, void __unused__ *arg)
{
   int rc = VM_IGNORE;
   pci_cfg_addr_t *addr = &info->vm.dev.pci_addr;

   addr->raw = ((pci_cfg_addr_t*)data)->raw;

   /* debug(DEV_PCI, "pci io addr filter: b%d f%d d%d r%d\n" */
   /*    ,addr->bus, addr->fnc, addr->dev, addr->reg); */

#if defined(CONFIG_HAS_NET) && defined(CONFIG_HAS_E1000)
   if(rc == VM_IGNORE)
      rc = __dev_pci_filter_e1000(&info->hrd.dev.net.pci.addr, addr);
#endif

#if defined(CONFIG_HAS_EHCI) && defined(CONFIG_EHCI_2ND)
   if(rc == VM_IGNORE)
      rc = __dev_pci_filter_ehci(&info->hrd.dev.dbgp.pci.addr, addr);
#endif

   if(rc == VM_IGNORE)
      __allow_io(PCI_CONFIG_DATA);
   else
      __deny_io(PCI_CONFIG_DATA);

   return VM_DONE;
}

int dev_pci(io_insn_t *io)
{
   switch(io->port)
   {
   case PCI_CONFIG_ADDR: info->vm.dev.pci_filter = __dev_pci_filter_addr; break;
   case PCI_CONFIG_DATA: /* set by pci filter addr */ break;
   default             : info->vm.dev.pci_filter = NULL;
   }

   return dev_io_proxify_filter(io, info->vm.dev.pci_filter, NULL);
}
