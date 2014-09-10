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
** only handle 32 bits mem BARs
*/
int pci_read_bar(pci_cfg_val_t *pci, uint32_t idx)
{
   if(idx > 5)
      return 0;

   pci->addr.reg = PCI_CFG_BAR_OFFSET + idx*4;
   pci_cfg_read(pci);

   return !(pci->br.io || pci->br.type);
}

/*
** For each pci_check_xxx() :
**
** - pci->addr should point to a valid function
** - pci->data will hold the matched value after return
*/
int pci_check_cap(pci_cfg_val_t *pci, uint32_t val)
{
   pci_cfg_cap_t cap = { .raw = val };

   pci->addr.reg = PCI_CFG_CMD_STS_OFFSET;
   pci_cfg_read(pci);

   if(pci->cs.sts.cap_list)
   {
      uint32_t next_cap;

      pci->addr.reg = PCI_CFG_CAP_OFFSET;
      pci_cfg_read(pci);
      next_cap =  pci->data.raw & 0xfc;

      while(next_cap)
      {
	 pci->addr.reg = next_cap;
	 pci_cfg_read(pci);

	 if(pci->cp.id == cap.id)
	    return 1;

	 next_cap = pci->cp.next & 0xfc;
      }
   }

   return 0;
}

int pci_check_dvd(pci_cfg_val_t *pci, uint32_t val)
{
   return (pci->dv.raw == val);
}

/*
** apply match() to every fn/dev/bus found
** returns 'nth' matching one if possible
**
** pci->data might be changed by match()
*/
int pci_search(pci_search_t match, uint32_t val, size_t nth, pci_cfg_val_t *pci)
{
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
	    pci_cfg_read(pci);

	    if(pci->dv.vendor == 0xffff)
	       continue;

	    if(match(pci, val))
	    {
	       debug(PCI, "pci match b%d d%d f%d r%d = 0x%x\n"
		     ,pci->addr.bus, pci->addr.dev, pci->addr.fnc
		     ,pci->addr.reg, pci->data.raw);

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

void pci_dump_registers(pci_cfg_val_t *pci)
{
   size_t n, r;

   for(n=0 ; n<16 ; n++)
   {
      r = n*4;
      pci->addr.reg = r;
      pci_cfg_read(pci);
      debug(PCI_E1000, "pci reg #%d 0x%x: 0x%x\n", n, r, pci->data.raw);
   }
}

