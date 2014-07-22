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
#include <debug.h>

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
	    debug(PCI, "pci b%d d%d f%d r%d = 0x%x\n"
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
** returns 'nth' matching one if possible
**
** pci->data might be changed by match()
*/
int pci_search(pci_search_t match, size_t nth, pci_cfg_val_t *pci)
{
   pci_cfg_dev_vdr_t dvd;
   pci_cfg_val_t     pci_bk;
   uint32_t          bus, dev, fnc;
   size_t            found;

   found = 0;
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
	    {
	       if(++found == nth)
		  return 1;

	       pci_bk.addr.raw = pci->addr.raw;
	       pci_bk.data.raw = pci->data.raw;
	    }
	 }
      }
   }

   if(found)
   {
      pci->addr.raw = pci_bk.addr.raw;
      pci->data.raw = pci_bk.data.raw;
      return 1;
   }

   return 0;
}

