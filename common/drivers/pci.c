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
#include <pci.h>
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
   pci_cfg_dbg_cap_t dbg_cap;
   pci_cfg_val_t     *pci = &dbgp_i->pci;
   loc_t             loc;

   dbgp_i->port = DBGP_INVALID;
   dbgp_i->addr = 0;

   pci->data.raw = PCI_DBGP_CAP_ID;
   if(!pci_search(pci_check_cap, pci))
      panic("no ehci debug port found");

   dbg_cap.raw = pci->data.raw;

   if(!pci_read_mm_bar(pci, dbg_cap.nr))
      panic("could not get ehci registers");

   loc.linear = pci->data.raw;
   dbgp_i->ehci_cap = (ehci_host_cap_reg_t*)loc.addr;

   pci_cfg_dbgp_vendor_specific(dbgp_i);

   if(dbgp_i->port == DBGP_INVALID)
      dbgp_i->port = dbgp_i->ehci_cap->hcs.dbg_nr;

   if(!dbgp_i->port)
      panic("could not get portsc for debug port");

   loc.linear = pci->data.raw + dbgp_i->ehci_cap->length;
   dbgp_i->ehci_opr = (ehci_host_op_reg_t*)loc.addr;

   loc.linear = pci->data.raw + dbg_cap.offset;
   dbgp_i->ehci_dbg = (ehci_dbgp_reg_t*)loc.addr;

   dbgp_i->ehci_psc = &dbgp_i->ehci_opr->portsc[dbgp_i->port - 1];
}

void pci_cfg_dbgp_vendor_specific(dbgp_info_t *dbgp_i)
{
   pci_cfg_dev_vdr_t dvd;
   pci_cfg_val_t     *pci = &dbgp_i->pci;

   pci->addr.reg = PCI_CFG_DEV_VDR_OFFSET;
   dvd.raw = pci_cfg_read(pci->addr);

   switch(dvd.vendor)
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
   uint32_t      mcp51;
   pci_cfg_val_t *pci = &dbgp_i->pci;

   dbgp_i->port = 2; /* XXX: we should identify ports and let user choose */

   pci->addr.reg = 0x74;
   mcp51 = pci_cfg_read(pci->addr);

   mcp51 &= ~(0xf<<12);
   mcp51 |=  (dbgp_i->port & 0xf)<<12;

   pci_cfg_write(pci->addr, mcp51);
}

/*
** pci->addr should be initialized
** pci->data will receive BAR addr
**
** only handle 32 bits mem BARs
*/
int pci_read_bar(pci_cfg_val_t *pci, uint32_t nr, uint8_t io)
{
   pci_cfg_bar_mem_t bar;

   if(nr<1 || nr>6)
      return 0;

   pci->addr.reg = PCI_CFG_BAR_OFFSET + (nr - 1)*4;
   bar.raw = pci_cfg_read(pci->addr);

   if(io)
      return 0;

   if(bar.io != io)
      return 0;

   if(!bar.io && bar.type)
      return 0;

   pci->data.raw = bar.addr<<8;
   return 1;
}

/*
** pci->addr should be initialized
** pci->data should be set to CAP_ID to check
*/
int pci_check_cap(pci_cfg_val_t *pci)
{
   pci_cfg_cmd_sts_t cmd_sts;

   pci->addr.reg = PCI_CFG_CMD_STS_OFFSET;
   cmd_sts.raw = pci_cfg_read(pci->addr);

   if(cmd_sts.sts.cap_list)
   {
      uint32_t      next_cap;
      pci_cfg_cap_t cap;

      pci->addr.reg = PCI_CFG_CAP_OFFSET;
      next_cap = pci_cfg_read(pci->addr) & 0xfc;

      while(next_cap)
      {
	 pci->addr.reg = next_cap;
	 cap.raw = pci_cfg_read(pci->addr);

	 if(cap.id == pci->data.blow)
	 {
	    pci->data.raw = cap.raw;
	    debug(EHCI, "pci b%d d%d f%d r%d = 0x%x\n"
		  ,pci->addr.bus, pci->addr.dev, pci->addr.fnc, pci->addr.reg
		  ,pci->data.raw);
	    return 1;
	 }

	 next_cap = cap.next & 0xfc;
      }
   }

   return 0;
}

/*
** apply match() to every fnc of every dev found
**
** pci->data might be changed by match()
*/
int pci_search(pci_search_t match, pci_cfg_val_t *pci)
{
   pci_cfg_dev_vdr_t dvd;
   uint32_t          bus, dev, fnc;
#ifdef __EHCI_2ND__
   int               found=0;
   pci_cfg_val_t     pci_bk;
#endif

   pci->addr.raw  = 0;
   pci->addr.enbl = 1;

   for(bus=0 ; bus<256 ; bus++)
   {
      pci->addr.bus = bus;

      for(dev=0 ; dev<32 ; dev++)
      {
	 pci->addr.dev = dev;

	 for(fnc=0 ; fnc<8 ; fnc++)
	 {
	    pci->addr.fnc = fnc;

	    pci->addr.reg = PCI_CFG_DEV_VDR_OFFSET;
	    dvd.raw = pci_cfg_read(pci->addr);

	    if(dvd.vendor == 0xffff)
	       continue;

	    if(match(pci))
#ifdef __EHCI_2ND__
	    {
	       if(++found == 2)
		  return 1;

	       pci_bk.addr.raw = pci->addr.raw;
	       pci_bk.data.raw = pci->data.raw;
	    }
#else
	       return 1;
#endif
	 }
      }
   }
#ifdef __EHCI_2ND__
   if(found)
   {
      pci->addr.raw = pci_bk.addr.raw;
      pci->data.raw = pci_bk.data.raw;
      return 1;
   }
#endif
   return 0;
}

