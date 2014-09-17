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
#include <pci_e1000.h>
#include <e1000.h>
#include <net.h>
#include <debug.h>

void pci_cfg_e1k(net_info_t *net)
{
   pci_cfg_dev_vdr_t dvd;
   e1k_info_t        *e1k = &net->arch;
   pci_cfg_val_t     *pci = &net->pci;

   /* search dev */
   dvd.vendor = PCI_CFG_VENDOR_INTEL;
   dvd.device = PCI_CFG_DEVICE_i82545EM_C;
   if(!pci_search(pci_check_dvd, dvd.raw, 1, pci))
   {
      dvd.device = PCI_CFG_DEVICE_i82540EM;
      if(!pci_search(pci_check_dvd, dvd.raw, 1, pci))
	 goto __not_found;
   }

   pci->addr.reg = PCI_CFG_CLASS_REV_OFFSET;
   pci_cfg_read(pci);
   if(pci->cr.class != PCI_CFG_CLASS_i8254x)
      goto __not_found;

   pci->addr.reg = PCI_CFG_CMD_STS_OFFSET;
   pci_cfg_read(pci);

   /* dma enable */
   if(!pci->cs.cmd.mm || !pci->cs.cmd.bus_master)
   {
      pci->cs.cmd.mm = 1;
      pci->cs.cmd.bus_master = 1;
      pci_cfg_write(pci);
   }

   pci_cfg_read(pci);
   debug(PCI_E1000, "e1k mm %d dma %d\n", pci->cs.cmd.mm, pci->cs.cmd.bus_master);

   /* BAR regs */
   if(!pci_read_bar(pci, 0) && pci->br.io)
      panic("could not read i8254x registers (bar0 0x%x)", pci->br.raw);

   e1k->base.linear = pci->br.raw & 0xfffffff0;

   if(pci->br.type == PCI_CFG_MEM_BAR_64)
   {
      raw64_t addr = { .low = 0 };

      pci_read_bar(pci, 1);
      addr.high = pci->data.raw;
      e1k->base.linear += addr.raw;
   }

   debug(PCI_E1000, "e1k BAR 0x%X\n", e1k->base.linear);

   setptr(e1k->ctl,    e1k->base.linear + 0);
   setptr(e1k->sts,    e1k->base.linear + 8);
   setptr(e1k->eerd,   e1k->base.linear + 0x14);
   setptr(e1k->fcal,   e1k->base.linear + 0x28);
   setptr(e1k->fcah,   e1k->base.linear + 0x2c);
   setptr(e1k->fct,    e1k->base.linear + 0x30);
   setptr(e1k->fcttv,  e1k->base.linear + 0x170);

   setptr(e1k->icr,    e1k->base.linear + 0xc0);
   setptr(e1k->ics,    e1k->base.linear + 0xc8);
   setptr(e1k->ims,    e1k->base.linear + 0xd0);
   setptr(e1k->imc,    e1k->base.linear + 0xd8);

   setptr(e1k->rx.ctl, e1k->base.linear + 0x100);
   setptr(e1k->rx.bal, e1k->base.linear + 0x2800);
   setptr(e1k->rx.bah, e1k->base.linear + 0x2804);
   setptr(e1k->rx.dl,  e1k->base.linear + 0x2808);
   setptr(e1k->rx.dh,  e1k->base.linear + 0x2810);
   setptr(e1k->rx.dt,  e1k->base.linear + 0x2818);

   setptr(e1k->tx.ctl, e1k->base.linear + 0x400);
   setptr(e1k->tx.ipg, e1k->base.linear + 0x410);
   setptr(e1k->tx.bal, e1k->base.linear + 0x3800);
   setptr(e1k->tx.bah, e1k->base.linear + 0x3804);
   setptr(e1k->tx.dl,  e1k->base.linear + 0x3808);
   setptr(e1k->tx.dh,  e1k->base.linear + 0x3810);
   setptr(e1k->tx.dt,  e1k->base.linear + 0x3818);

   setptr(e1k->mta,    e1k->base.linear + 0x5200);
   setptr(e1k->ra,     e1k->base.linear + 0x5400);

   /* Interrupt line */
   pci->addr.reg = PCI_CFG_INT_OFFSET;
   pci_cfg_read(pci);

   net->irq = pci->data.blow;

   debug(PCI_E1000, "e1k irq line %d\n", net->irq);

   return;

__not_found:
   panic("no i8254x ethernet controller found");
}
