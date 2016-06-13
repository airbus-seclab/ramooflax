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
#include <mp.h>
#include <apic.h>
#include <asm.h>
#include <debug.h>
#include <string.h>
#include <realmem.h>

static mp_ptr_t* mp_search_ptr_chunk(uint32_t addr, uint32_t len)
{
   uint32_t dst, src;
   mp_ptr_t *mp_ptr;
   loc_t    loc;

   src = addr;

   while(len)
   {
      _memchr8(dst, src, 0x5f, len);
      if(!len)
         break;

      loc.linear = dst - 1;
      mp_ptr = (mp_ptr_t*)loc.addr;
      if(mp_ptr->sig == MP_PTR_SIG)
         return mp_ptr;

      len -= dst - src;
      src  = dst;
   }

   return (mp_ptr_t*)0;
}

static mp_ptr_t* mp_search_ptr()
{
   mp_ptr_t *mp_ptr;
   uint32_t len;

   len = 1024 - sizeof(mp_ptr_t);

   if((mp_ptr=mp_search_ptr_chunk(BIOS_EXT_DATA_START, len)))
      return mp_ptr;

   if((mp_ptr=mp_search_ptr_chunk(LOW_MEM_END - len, len)))
      return mp_ptr;

   len = BIOS_END - BIOS_START - sizeof(mp_ptr_t);
   if((mp_ptr=mp_search_ptr_chunk(BIOS_START, len)))
      return mp_ptr;

   return 0;
}

static void mp_parse(mp_table_t *mp_tbl)
{
   loc_t    loc;
   uint32_t nr;

   nr = mp_tbl->count;

   loc.addr = mp_tbl;
   loc.linear += sizeof(mp_table_t);

   while(nr--)
   {
      if(*loc.u8 == MP_TBL_ENTRY_PROC)
      {
         mp_tbl_proc_t *proc = (mp_tbl_proc_t*)loc.addr;
         debug(MP, "proc entry: en %d bp %d\n", proc->flags.en, proc->flags.bp);
         loc.linear += sizeof(mp_tbl_proc_t);
      }
      else
      {
         debug(MP, "entry %d\n", *loc.u8);
         loc.linear += MP_TBL_ENTRY_SZ_OTHER;
      }
   }
}

void mp_init()
{
   loc_t      loc;
   mp_ptr_t   *mp_ptr;
   mp_table_t *mp_tbl;

   mp_ptr = (mp_ptr_t*)mp_search_ptr();
   if(!mp_ptr || mp_ptr->features[0] != 0)
      panic("no mp table present\n");

   if(mp_ptr->features[1] & MP_PTR_FEAT_1_IMCR)
      imcr_set_apic();

   loc.linear = mp_ptr->addr;
   mp_tbl = (mp_table_t*)loc.addr;

   if(mp_tbl->sig != MP_TBL_SIG)
   {
      debug(MP, "bad mp table found\n");
      return;
   }

   debug(MP, "MP table @ 0x%x #%d\n", mp_ptr->addr, mp_tbl->count);
   mp_parse(mp_tbl);
}

