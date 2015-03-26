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
#include <smap.h>
#include <vmm.h>
#include <string.h>
#include <print.h>
#include <debug.h>

void smap_parse(mbi_t *mbi, smap_t *smap, offset_t *area, offset_t *acme)
{
   loc_t         mmap;
   memory_map_t  *mme;
   smap_e_t      *sme;
   offset_t      top;

   mmap.linear = mbi->mmap_addr;

   top   = 0;
   *area = 0;
   *acme = TOP_4GB;

   while(mmap.linear < mbi->mmap_addr + mbi->mmap_length)
   {
      mme = (memory_map_t*)mmap.addr;
      sme = (smap_e_t*)&mme->addr;

      debug(SMAP,"smap entry: base 0x%X | len 0x%X | type %d\n",
	    sme->base, sme->len, sme->type );

      if(mme->size > sizeof(smap_e_t))
	 panic("smap entry too big");

      top = sme->base + sme->len;

      if(sme->type == SMAP_TYPE_AVL && sme->base == 0x100000ULL)
	 *area = top;

      mmap.linear += mme->size + sizeof(mme->size);
      smap->nr++;
   }

   if(*area < VMM_MIN_RAM_SZ)
      panic("not enough memory");

   if(top > TOP_4GB)
      *acme = top;
}

void smap_init(mbi_t *mbi, smap_t *smap, offset_t top)
{
   loc_t        mmap;
   memory_map_t *mme;
   smap_e_t     *sme_i, *sme_o;

   mmap.linear = mbi->mmap_addr;
   sme_o       = &smap->entries[0];

   while(mmap.linear < mbi->mmap_addr + mbi->mmap_length)
   {
      mme   = (memory_map_t*)mmap.addr;
      sme_i = (smap_e_t*)&mme->addr;

      memcpy((void*)sme_o, (void*)sme_i, sizeof(smap_e_t));

      if(sme_o->type == SMAP_TYPE_AVL && sme_o->base == 0x100000ULL)
	 sme_o->len = top - sme_o->base;

      mmap.linear += mme->size + sizeof(mme->size);
      sme_o++;
   }
}

