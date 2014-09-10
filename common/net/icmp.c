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
#include <icmp.h>
#include <string.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

size_t icmp_echo_request(icmp_hdr_t *hdr)
{
   loc_t      data;
   size_t     dlen, len;

   hdr->type = ICMP_TYPE_ECHO_REQUEST;
   hdr->code = 0;

   hdr->ping.id  = swap16(0xcafe);
   hdr->ping.seq = swap16(0);

   data.addr = hdr;
   data.linear += sizeof(icmp_hdr_t);

   dlen = 4;
   memset(data.addr, 'a', dlen);

   len = sizeof(icmp_hdr_t) + dlen;
   icmp_checksum(hdr, len);

   debug(ICMP, "icmp pkt len %d (hdr %d)\n", len, sizeof(icmp_hdr_t));
   return len;
}
