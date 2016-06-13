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
#include <checksum.h>
#include <insn.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

uint16_t rfc1071_checksum(uint16_t *data, size_t len)
{
   raw32_t chk = {.raw = 0};

   len /= 2;
   while(len--)
      chk.raw += swap16(*data++);

   return ~adc16(chk.whigh, chk.wlow);
}
