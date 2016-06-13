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
#include <pool.h>
#include <pagemem.h>
#include <info_data.h>

extern info_data_t *info;

#ifdef __INIT__
void pool_init()
{
   size_t i, nr;

   nr = info->vmm.pool.sz/PAGE_SIZE;
   for(i=0 ; i<nr ; i++)
   {
      info->vmm.pool.all[i].page.raw = info->vmm.pool.addr + i*PAGE_SIZE;
      cdll_push(info->vmm.pool.free, &info->vmm.pool.all[i]);
   }
}
#endif

static inline pool_pg_desc_t* pool_get_desc(offset_t pg)
{
   size_t mn, mx, nr;

   nr = page_nr(pg);
   mn = page_nr(info->vmm.pool.addr);
   mx = page_nr(info->vmm.pool.addr + info->vmm.pool.sz);

   if(nr < mn || nr >= mx)
      return (pool_pg_desc_t*)0;

   return &info->vmm.pool.all[nr-mn];
}

void pool_push_page(offset_t pg)
{
   pool_pg_desc_t *desc = pool_get_desc(pg);

   if(!desc || !(desc->page.flags & POOL_PG_INUSE))
      return;

   cdll_del(info->vmm.pool.used, desc);
   desc->page.flags &= ~POOL_PG_INUSE;
   cdll_push(info->vmm.pool.free, desc);
}

offset_t pool_pop_page()
{
   pool_pg_desc_t *desc = cdll_pop(info->vmm.pool.free);

   if(!desc)
      return (offset_t)0;

   desc->page.flags |= POOL_PG_INUSE;
   cdll_push(info->vmm.pool.used, desc);
   return (desc->page.raw & ~((offset_t)(PAGE_SIZE-1)));
}
