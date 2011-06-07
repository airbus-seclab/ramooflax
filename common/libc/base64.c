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
#include <base64.h>
#include <ehci.h>

static uint8_t table[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline void __base64_encode(uint64_t val, uint8_t *out)
{
   size_t i;

   val = swap64(val)>>16;

   for(i=0 ; i<8 ; i++)
   {
      out[7-i] = table[ val & 0x3f ];
      val >>= 6;
   }
}

void base64_encode(offset_t addr, size_t len, buffer_t *out)
{
   buffer_t in;
   size_t   i, rm;

   out->sz = 0;
   in.sz = len/6;

   in.data.linear = addr;
   rm = len%6;
   for(i=0 ; i<in.sz ; i++, in.data.linear+=6)
   {
      __base64_encode(*in.data.u64, &out->data.u8[out->sz]);
      out->sz += 8;
   }

   if(rm)
   {
      __base64_encode(*in.data.u64, &out->data.u8[out->sz]);

      if(rm > 3) rm += 2;
      else       rm += 1;

      out->data.u8[out->sz+rm] = '=';
      out->sz += rm+1;
   }
}
