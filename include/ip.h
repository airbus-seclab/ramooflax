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
#ifndef __IP_H__
#define __IP_H__

#include <types.h>
#include <checksum.h>
#include <insn.h>

#define IP_PROTO_ICMP 0x01
#define IP_PROTO_UDP  0x11

typedef union ip_header_fragment
{
   struct
   {
      uint16_t  off:13;
      uint16_t  mf:1;
      uint16_t  df:1;
      uint16_t  zf:1;

   } __attribute__((packed));

   uint16_t raw;

} __attribute__((packed)) ip_hdr_frag_t;

typedef uint32_t ip_addr_t;
#define IP_STR_SZ (sizeof(ip_addr_t)*4)

typedef union ip_header
{
   struct
   {
      uint32_t      ihl:4;
      uint32_t      ver:4;
      uint32_t      ecn:2;
      uint32_t      dscp:6;
      uint16_t      len;
      uint16_t      id;
      ip_hdr_frag_t frag;
      uint8_t       ttl;
      uint8_t       proto;
      uint16_t      chk;
      ip_addr_t     src;
      ip_addr_t     dst;

   } __attribute__((packed));

   uint8_t raw[20];

} __attribute__((packed)) ip_hdr_t;

/*
** Functions
*/
#define ip_checksum(_hdr_)						\
   ({									\
      (_hdr_)->chk = 0;							\
      (_hdr_)->chk = rfc1071_checksum((uint16_t*)(_hdr_), sizeof(ip_hdr_t)); \
   })

size_t ip_icmp_pkt(ip_hdr_t*, size_t);
size_t ip_udp_pkt(ip_hdr_t*, size_t);
void   ip_dissect(loc_t, size_t);
void   ip_str(ip_addr_t, char*);

#endif
