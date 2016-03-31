/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#include <ppg.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

#ifdef __INIT__
void ppg_desc_init()
{
   ppg_info_t *ppg  = &info->hrd.mem.ppg;
   ppg_dsc_t  *dsc  = ppg->dsc;
   offset_t    end  = info->hrd.mem.top;
   offset_t    addr = 0;

   ppg->vmm.nr = 0;
   ppg->vm.nr  = 0;

   while(addr < end)
   {
      dsc->addr = addr;

      if(vmm_area(addr))
      {
	 dsc->count = 1;
	 dsc->vmm   = 1;
	 cdll_fill(ppg->vmm.list, dsc);
	 ppg->vmm.nr++;
      }
      else
      {
	 cdll_fill(ppg->vm.list, dsc);
	 ppg->vm.nr++;
      }

      addr += PAGE_SIZE;
      dsc++;
   }
}
#endif

ppg_dsc_t* ppg_get_desc(offset_t addr)
{
   ppg_info_t *ppg = &info->hrd.mem.ppg;
   offset_t    n   = page_nr(addr);

   if(n < ppg->nr)
      return &ppg->dsc[n];

   return ((ppg_dsc_t*)0);
}
