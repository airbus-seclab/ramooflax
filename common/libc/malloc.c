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
#include <malloc.h>
#include <slab.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

void* malloc(size_t sz)
{
   slab_t     *slab;
   slab_obj_t *obj;

   if(!(slab = slab_find(sz)) && !(slab = slab_new(sz, 1)))
   {
      debug(VMEM, "malloc: can't find/create new slab\n");
      return (void*)0;
   }

   if(!(obj = cdll_pop(slab->obj)))
   {
      if(!slab_grow(slab, 1))
      {
	 debug(VMEM, "malloc: slab exhausted/can't grow\n");
	 return (void*)0;
      }

      obj = cdll_pop(slab->obj);
   }

   slab->nr--;
   return (void*)obj;
}
