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
#ifndef __INFO_DATA_H__
#define __INFO_DATA_H__

#include <types.h>
#include <vmm.h>
#include <vm.h>
#include <dev.h>

/*
** Information data header
*/
#define __info_data_hdr__                       \
   __attribute__ ((section(".info_hdr")))

#define mapped_area(_addr)                      \
   ((offset_t)(_addr) < info->hrd.mem.top)

#define vmm_area(_addr)                                         \
   mem_range((offset_t)(_addr),info->area.start,info->area.end)

#define vmm_area_range(_addr, _sz)                              \
   ({                                                           \
      int rc = 1;                                               \
      if((size_t)_sz < 2)                                       \
         rc = vmm_area(_addr)?1:0;                              \
      else                                                      \
      {                                                         \
         offset_t last = (offset_t)_addr + (size_t)_sz - 1;     \
         if((offset_t)_addr < last)                             \
            rc = (vmm_area(_addr) && vmm_area(last))?1:0;       \
      }                                                         \
      rc;                                                       \
   })

/*
** The knowledge is here dude !
*/
typedef struct the_secret_area
{
   offset_t start;
   offset_t end;
   size_t   size;

} __attribute__((packed)) area_t;

typedef struct information_data
{
   hrdw_t  hrd;   /* hardware data */
   vmm_t   vmm;   /* virtual machine monitor data */
   vm_t    vm;    /* virtual machine data */
   area_t  area;  /* invisible area size */

} __attribute__((packed)) info_data_t;


#endif
