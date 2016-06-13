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
#include <slab.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** Elementary slab allocator
**
** - cache grow is based on pool pages
** - cache obj size is rounded with slab_size(sz)
** - cache objects are aligned on cache size (sz <= PAGE_SIZE)
**
** Heap is allocated through vmm pool, meaning that heap addr are
** those retrieved from pool pages.
**
** We request one page at a time because we are not sure that multiple
** pool_pop_page() give contiguous pages in memory.  VMM memory is
** identity mapped and no special heap area remaps pool pages to not
** waste memory space.
**
*/
static offset_t slab_req_page(slab_t *slab)
{
   offset_t addr = pool_pop_page();

   if(addr)
   {
      ppg_dsc_t *ppgd = ppg_get_desc(addr);

      if(!ppgd)
         return 0;

      ppgd->slab = slab;
   }

   return addr;
}

static void slab_setup(slab_t *slab, size_t sz)
{
   slab->obj = NULL;
   slab->nr  = 0;
   slab->sz  = sz;
}

static slab_t* slab_list_grow()
{
   size_t  i, nr;
   slab_t *slab = (slab_t*)pool_pop_page();

   if(!slab)
      return slab;

   nr = PAGE_SIZE/sizeof(slab_t);
   for(i=0 ; i<nr ; i++, slab++)
   {
      slab_setup(slab, 0);
      cdll_fill(info->vmm.slab.avl, slab);
   }

   debug(SLAB, "slab list grow +%D\n", nr);
   return slab;
}

static slab_t* slab_alloc()
{
   if(!info->vmm.slab.avl && !slab_list_grow())
   {
      debug(SLAB, "could not grow avl slab list\n");
      return (slab_t*)0;
   }

   return cdll_pop(info->vmm.slab.avl);
}

static slab_t* slab_create(size_t sz, size_t nr)
{
   slab_t *slab = slab_alloc();

   if(!slab)
   {
      debug(SLAB, "could not allocate new slab\n");
      return (slab_t*)0;
   }

   slab_setup(slab, sz);

   if(!slab_grow(slab, nr))
   {
      cdll_fill(info->vmm.slab.avl, slab);
      return (slab_t*)0;
   }

   debug(SLAB, " + new cache create done (%D)\n", sz);
   return slab;
}

static slab_t* slab_insert(size_t sz, size_t nr, slab_t *nxt)
{
   slab_t *slab = slab_create(sz, nr);

   if(!slab)
      return (slab_t*)0;

   if(nxt)
      cdll_insert_before(info->vmm.slab.used, slab, nxt);
   else
      cdll_fill(info->vmm.slab.used, slab);

   debug(SLAB, " + added new cache (%D) to cache list\n", sz);
   info->vmm.slab.nr++;
   return slab;
}

/* static size_t __slab_size_power2(size_t asz) */
/* { */
/*    size_t sh = 4; */
/*    size_t sz = 1ULL<<sh; */

/*    while(sz < asz) */
/*       sz = _2_power(++sh); */

/*    return sz; */
/* } */

static size_t __slab_size_long(size_t asz)
{
   if(long_aligned(asz))
      return asz;

   return (size_t)long_align_next(asz);
}

static size_t slab_size(size_t asz)
{
   if(asz < sizeof(slab_obj_t))
      return sizeof(slab_obj_t);

   return __slab_size_long(asz);
   /* return __slab_size_power2(asz); */
}

slab_t* slab_at(offset_t addr)
{
   ppg_dsc_t *ppgd = ppg_get_desc(addr);

   if(ppgd)
      return ppgd->slab;

   return (slab_t*)0;
}

void slab_add(slab_t *slab, slab_obj_t *obj)
{
   if(!slab)
      return;

   if(cdll_search(slab->obj, obj))
      return;

   cdll_fill(slab->obj, obj);
   slab->nr++;
}

size_t slab_grow(slab_t *slab, size_t nr)
{
   size_t   bytes, pages, i, np;
   offset_t addr;

   if(!slab || !nr)
      return 0;

   bytes = slab->sz * nr;
   pages = bytes >> PAGE_SHIFT;
   if(bytes % PAGE_SIZE)
      pages++;

   np = PAGE_SIZE/slab->sz;
   nr = 0;
   while(pages--)
   {
      addr = slab_req_page(slab);
      if(!addr)
         goto __no_more;

      for(i=0 ; i<np ; i++, addr += slab->sz)
         cdll_fill(slab->obj, (slab_obj_t*)addr);

      slab->nr += np;
      nr += np;
   }

__no_more:
   debug(SLAB, " + slab grown with %D objects\n", nr);
   return nr;
}

slab_t* slab_find(size_t asz)
{
   slab_t *slab = info->vmm.slab.used;
   size_t  sz   = slab_size(asz);

   if(!slab)
      return (slab_t*)0;

   while(slab->next != info->vmm.slab.used && sz > slab->sz)
      slab = slab->next;

   if(slab->sz == sz)
      return slab;

   return (slab_t*)0;
}

slab_t* slab_new(size_t asz, size_t nr)
{
   slab_t *slab;
   size_t  sz;

   /* XXX: limit */
   if(asz > PAGE_SIZE/2)
      return (slab_t*)0;

   sz = slab_size(asz);
   debug(SLAB, "slab new cache: sz (%D -> %D) nr %D\n", asz, sz, nr);

   slab = info->vmm.slab.used;
   if(!slab)
      goto __biggest;

   while(slab->next != info->vmm.slab.used && sz > slab->sz)
      slab = slab->next;

   if(sz < slab->sz)
      return slab_insert(sz, nr, slab);
   else if(sz == slab->sz)
   {
      if(!slab_grow(slab, nr))
         return (slab_t*)0;

      debug(SLAB, " + cache extended to %D obj\n", slab->nr);
      return slab;
   }

   /* biggest one */
__biggest:
   return slab_insert(sz, nr, NULL);
}
