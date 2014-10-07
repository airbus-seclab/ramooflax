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

static int __dev_pci_dvd_filter(void *data)
{
   pci_cfg_dev_vdr_t *dvd = (pci_cfg_dev_vdr_t*)data;

   debug(DEV_PCI, "pci io dvd filter: 0x%x\n", dvd->raw);

   if(dvd->vendor != PCI_CFG_VENDOR_INTEL)
      return 0;

   if(dvd->device != PCI_CFG_DEVICE_i82540EM &&
      dvd->device != PCI_CFG_DEVICE_i82545EM_C)
      return 0;

   debug(DEV_PCI, "pci io filter: ninja netcard\n");
   dvd->device = PCI_CFG_DEVICE_INVALID;
   return 1;
}

static int __dev_pci_addr_filter(void *data)
{
   pci_cfg_addr_t *addr = (pci_cfg_addr_t*)data;
   pci_cfg_addr_t *net = &info->hrd.dev.net.pci.addr;

   debug(DEV_PCI, "pci io addr filter: b%d f%d d%d r%d\n"
	 ,addr->bus, addr->fnc, addr->dev, addr->reg);

   if(addr->bus == net->bus &&
      addr->fnc == net->fnc &&
      addr->dev == net->dev &&
      addr->reg == PCI_CFG_DEV_VDR_OFFSET)
   {
      debug(DEV_PCI, "pci io filter: dvd request\n");
      __deny_io(PCI_CONFIG_DATA);
      return 1;
   }

   __allow_io(PCI_CONFIG_DATA);
   return 0;
}

int dev_pci(io_insn_t *io)
{
   io_flt_hdl_t filter = NULL;

   switch(io->port)
   {
   case PCI_CONFIG_ADDR: filter = __dev_pci_addr_filter; break;
   case PCI_CONFIG_DATA: filter = __dev_pci_dvd_filter;  break;
   }

   return dev_io_proxify_filter(io, filter);
}
