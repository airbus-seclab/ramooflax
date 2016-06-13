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
#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <types.h>
#include <slab.h>
#include <string.h>


/*
** Functions
*/
#define free(_aDdR) slab_add(slab_at((offset_t)(_aDdR)),(slab_obj_t*)(_aDdR))

void* malloc(size_t);

static inline void* calloc(size_t sz)
{
   void *m = malloc(sz);

   if(m)
      memset(m, 0, sz);

   return m;
}


#endif
