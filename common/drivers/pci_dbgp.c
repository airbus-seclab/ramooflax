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
#include <pci_dbgp.h>
#include <ehci.h>
#include <print.h>
#include <debug.h>

/*
** Retrieve EHCI Debug Port stuff :
**  - io mem registers to setup
**  - ehci controller and debug port
**  - board ids
*/
void pci_cfg_dbgp(dbgp_info_t *dbgp_i)
{
   pci_cfg_dbg_cap_t  dbg_cap;
   loc_t              base, loc;
   pci_cfg_val_t      *pci = &dbgp_i->pci;

   dbgp_i->port = DBGP_INVALID;
   dbgp_i->addr = 0;

   debug(PCI_DBGP, "pci dbgp init\n");

#ifdef CONFIG_EHCI_2ND
   if(!pci_search(pci_check_cap, PCI_DBGP_CAP_ID, 2, pci))
#else
   if(!pci_search(pci_check_cap, PCI_DBGP_CAP_ID, 1, pci))
#endif
      panic("no ehci debug port found");

   dbg_cap.raw = pci->data.raw;

   if(!pci_read_bar(pci, dbg_cap.nr-1))
      panic("could not get ehci registers");

   if(pci->br.type == PCI_CFG_MEM_BAR_64)
      panic("ehci 64 bits BAR not supported");

   base.high = 0;
   base.low  = pci->br.raw & 0xffffff00; /* ehci bar */
   dbgp_i->ehci_cap = (ehci_host_cap_reg_t*)base.addr;

   pci_cfg_dbgp_vendor_specific(dbgp_i);

   if(dbgp_i->port == DBGP_INVALID)
      dbgp_i->port = dbgp_i->ehci_cap->hcs.dbg_nr;

   if(!dbgp_i->port)
      panic("could not get portsc for debug port");

   loc.linear = base.linear + dbgp_i->ehci_cap->length;
   dbgp_i->ehci_opr = (ehci_host_op_reg_t*)loc.addr;

   loc.linear = base.linear + dbg_cap.offset;
   dbgp_i->ehci_dbg = (ehci_dbgp_reg_t*)loc.addr;

   dbgp_i->ehci_psc = &dbgp_i->ehci_opr->portsc[dbgp_i->port - 1];
}

void pci_cfg_dbgp_vendor_specific(dbgp_info_t *dbgp_i)
{
   pci_cfg_val_t *pci = &dbgp_i->pci;

   pci->addr.reg = PCI_CFG_DEV_VDR_OFFSET;
   pci_cfg_read(pci);

   switch(pci->dv.vendor)
   {
   case PCI_CFG_VENDOR_NVIDIA:
      pci_cfg_dbgp_nvidia(dbgp_i);
      break;
   default:
      break;
   }
}

/*
** nForce 430 MCP51 allows to set the default value
** for debug port number found into ehci registers
**
** By default the value is the same in this register
** and in the ehci registers, that is #1 (0), but on the
** development board the usb port 0 is not accessible.
**
** Using this register you can change the default
** value found into ehci registers on next power-up.
**
** This value will be reset when AC is physically
** unplugged.
*/
void pci_cfg_dbgp_nvidia(dbgp_info_t *dbgp_i)
{
   pci_cfg_val_t *pci = &dbgp_i->pci;

   dbgp_i->port = 2; /* XXX: we should identify ports and let user choose */

   pci->addr.reg = 0x74;
   pci_cfg_read(pci);

   pci->data.raw &= ~(0xf<<12);
   pci->data.raw |=  (dbgp_i->port & 0xf)<<12;

   pci_cfg_write(pci);
}
