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
#ifndef __SLAB_H__
#define __SLAB_H__

#include <types.h>


/* An object is an area of contiguous bytes in virtual memory */
typedef struct slab_object
{
   struct slab_object *prev;
   struct slab_object *next;

} __attribute__((packed)) slab_obj_t;

/* A cache is a list of objects of the same size */
typedef struct slab_cache
{
   size_t      nr;    /* number of available objects in that cache */
   size_t      sz;    /* size of each object in that cache */
   slab_obj_t *obj;   /* available objects of that cache */

   struct slab_cache *prev;
   struct slab_cache *next;

} __attribute__((packed)) slab_t;

/* Cache info keeps track of all slab caches */
typedef struct slab_cache_info
{
   size_t  nr;   /* number of caches */
   slab_t *used; /* in-use caches list  */
   slab_t *avl;  /* pre-allocated caches list */

} __attribute__((packed)) vmm_slab_t;


/*
** Functions
*/
slab_t* slab_new(size_t, size_t);
size_t  slab_grow(slab_t*, size_t);
slab_t* slab_find(size_t);
slab_t* slab_at(offset_t);
void    slab_add(slab_t*, slab_obj_t*);


#endif
