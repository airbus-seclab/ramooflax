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
#include <video.h>
#include <string.h>

static uint32_t line, col;

static inline void scroll()
{
   uint16_t *nscreen = (uint16_t*)VGA_TXT_FB_ADDR;
   uint16_t *oscreen = (uint16_t*)VGA_TXT_FB_ADDR + VGA_TXT_C_NR;

   memcpy((void*)nscreen, (void*)oscreen, 2*VGA_TXT_C_NR*(VGA_TXT_L_NR - 1));
   memset((void*)&nscreen[VGA_TXT_C_NR*(VGA_TXT_L_NR - 1)], 0, VGA_TXT_C_NR*2);
   line = VGA_TXT_L_NR - 1;
}

static inline void newline()
{
   line++;
   if(line >= VGA_TXT_L_NR)
      scroll();
}

static inline void newcol()
{
   col++;
   col %= VGA_TXT_C_NR;
}

void video_write(uint8_t *buffer, size_t size)
{
   size_t   i;
   uint16_t *screen = (uint16_t*)VGA_TXT_FB_ADDR;

   for(i=0 ; i<size ; i++)
   {
      switch( buffer[i] )
      {
      case 0xa:
         newline();
      case 0xd:
         col = 0;
         break;
      default:
         screen[col+VGA_TXT_C_NR*line] = 0x0f00|buffer[i];
         newcol();
         if(! col)
            newline();
      }
   }
}
