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
#include <string.h>
#include <debug.h>

static char __hextable[] = {'0','1','2','3','4','5','6','7',
                            '8','9','a','b','c','d','e','f'};

void __uint8_to_hex(uint8_t *data, uint8_t value)
{
   data[0] = __hextable[(value>>4) & 0xf];
   data[1] = __hextable[ value     & 0xf];
}

size_t uint64_to_hex(buffer_t *buf, size_t len, uint64_t value, size_t precision)
{
   char   rep[sizeof(uint64_t)*2];
   size_t sz, rsz = 0;

   if(!precision || precision > 16)
      precision = -1;

   while(precision && !(precision > 16 && !value && rsz))
   {
      rep[rsz] = __hextable[value & 0xf];
      value >>= 4;
      rsz++;
      precision--;
   }

   sz = rsz;
   while(rsz--)
      __buf_add(buf, len, rep[rsz]);

   return sz;
}

static uint8_t __to_nibble(uint8_t nibble)
{
   if(nibble >= 'a' && nibble <= 'f')
   {
      nibble -= 'a'; goto __letter;
   }
   else if(nibble >= 'A' && nibble <= 'F')
   {
      nibble -= 'A'; goto __letter;
   }
   else if(nibble >= '0' && nibble <= '9')
   {
      nibble -= '0'; goto __valid;
   }

   return BAD_NIBBLE;

__letter:
   nibble += 10;

__valid:
   return nibble;
}

int __hex_to_uint8(uint8_t *data, uint8_t *value)
{
   uint8_t n1, n2;

   n1 = __to_nibble(data[1]);
   n2 = __to_nibble(data[0]);

   if(n1 == BAD_NIBBLE || n2 == BAD_NIBBLE)
      return 0;

   *value = n2<<4 | n1;
   return 1;
}

int hex_to_uint64(uint8_t *data, size_t len, uint64_t *value)
{
   uint64_t v = 0;
   size_t   shift = 0;

   if(!len)
      return 0;

   len = min(len, sizeof(uint64_t)*2);

   do
   {
      uint8_t nibble = __to_nibble(data[--len]);

      if(nibble == BAD_NIBBLE)
         return 0;

      v |= ((uint64_t)nibble)<<shift;
      shift += 4;

   } while(len);

   *value = v;
   return 1;
}

int dec_to_uint64(uint8_t *data, size_t len, uint64_t *value)
{
   uint64_t v = 0, f = 1;

   if(!len)
      return 0;

   len = min(len, 20);

   do
   {
      uint8_t x = data[--len];

      if(x < '0' || x > '9')
         return 0;

      x -= '0';
      v += x*f;
      f *= 10;

   } while(len);

   *value = v;
   return 1;
}
