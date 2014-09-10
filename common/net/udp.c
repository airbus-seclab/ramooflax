/*
** Copyright (C) 2014 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#include <udp.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

size_t udp_pkt(udp_hdr_t *hdr)
{
   loc_t   data;
   size_t  dlen, len;

   data.addr = hdr;
   data.linear += sizeof(udp_hdr_t);

   dlen = 4;
   memcpy(data.addr, "abcd", dlen);

   len = sizeof(udp_hdr_t) + dlen;

   hdr->src = swap16(0x1337);
   hdr->dst = swap16(2000);
   hdr->len = swap16(len);
   hdr->chk = 0;

   return len;
}
