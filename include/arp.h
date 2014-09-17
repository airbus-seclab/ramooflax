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
#ifndef __ARP_H__
#define __ARP_H__

#include <types.h>
#include <ether.h>
#include <ip.h>
#include <insn.h>

#define ARP_TYPE_ETH      1

#define ARP_OP_REQUEST    1
#define ARP_OP_REPLY      2

typedef union arp_header
{
   struct
   {
      uint16_t   hwtype;
      uint16_t   proto;
      uint8_t    hlen;
      uint8_t    plen;
      uint16_t   op;
      mac_addr_t hwsrc;
      ip_addr_t  psrc;
      mac_addr_t hwdst;
      ip_addr_t  pdst;

   } __attribute__((packed));

   uint8_t raw[28];

} __attribute__((packed)) arp_hdr_t;

typedef struct arp_cache_table_entry
{
   mac_addr_t mac;
   ip_addr_t  ip;

} __attribute__((packed)) arp_cache_t;

/* XXX: fix size to that of subnet mask */
#define ARP_CACHE_SZ  256

typedef struct arp_cache_table
{
   arp_cache_t  cache[ARP_CACHE_SZ];
   size_t       tail;

} __attribute__((packed)) arp_tbl_t;

/*
** Functions
*/
#ifdef __INIT__
void   arp_init();
#endif

typedef size_t (*arp_gen_t)(arp_hdr_t*,
			    mac_addr_t*, mac_addr_t*,
			    ip_addr_t, ip_addr_t);

size_t arp_who_has_pkt(arp_hdr_t*, mac_addr_t*, mac_addr_t*, ip_addr_t, ip_addr_t);
size_t arp_is_at_pkt(arp_hdr_t*, mac_addr_t*, mac_addr_t*, ip_addr_t, ip_addr_t);

int    arp_cache_lookup(ip_addr_t, mac_addr_t*);
int    arp_dissect(loc_t, size_t);

#endif
