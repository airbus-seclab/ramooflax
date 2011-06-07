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
#include <pf.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

/*
** Mirror map 1MB-2MB to 0MB-1MB if disabled
*/
void __pf_setup_a20()
{
   uint32_t i, offset;

   debug(PF
	 , "A20 %s @ 0x%x\n"
	 , info->vm.dev.mem.a20?"on":"off"
	 , __cs.base_addr.low + __rip.wlow );

   if(info->vm.dev.mem.a20)
      offset = 0;
   else
      offset = page_nr(RM_LIMIT);

   for(i=page_nr(RM_LIMIT) ; i<page_nr(RM_WRAP_LIMIT) ; i++)
      info->vm.cpu.pg.rm->pt[i].addr = i - offset;
}
