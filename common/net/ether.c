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
#include <ether.h>
#include <string.h>
#include <arp.h>
#include <ip.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

void mac_str(mac_addr_t *mac, char *str)
{
   snprintf(str, MAC_STR_SZ, "%x:%x:%x:%x:%x:%x"
	    ,mac->byte[0],mac->byte[1],mac->byte[2]
	    ,mac->byte[3],mac->byte[4],mac->byte[5]);
}

int mac_cmp(mac_addr_t *m1, mac_addr_t *m2)
{
   loc_t l1 = { .addr = m1 };
   loc_t l2 = { .addr = m2 };

   if(*l1.u32++ != *l2.u32++)
      return 0;

   if(*l1.u16 != *l2.u16)
      return 0;

   return 1;
}

void mac_copy(mac_addr_t *dst, mac_addr_t *src)
{
   loc_t l1 = { .addr = dst };
   loc_t l2 = { .addr = src };

   *l1.u32++ = *l2.u32++;
   *l1.u16   = *l2.u16;
}

void mac_rst(mac_addr_t *mac, uint8_t b)
{
   loc_t l1 = { .addr = mac };

   *l1.u32++ = __replicate_byte_on_dword(b);
   *l1.u16   = __replicate_byte_on_word(b);
}

void mac_set(mac_addr_t *mac, char *hex)
{
   mac_copy(mac, (mac_addr_t*)hex);
}

size_t __eth_gen(eth_hdr_t *hdr, mac_addr_t *src, mac_addr_t *dst)
{
   mac_copy(&hdr->src, src);
   mac_copy(&hdr->dst, dst);
   return sizeof(eth_hdr_t);
}

size_t eth_ip(eth_hdr_t *hdr, mac_addr_t *src, mac_addr_t *dst)
{
   hdr->type = swap16(ETHER_TYPE_IP);
   return __eth_gen(hdr, src, dst);
}

size_t eth_arp(eth_hdr_t *hdr, mac_addr_t *src, mac_addr_t *dst)
{
   hdr->type = swap16(ETHER_TYPE_ARP);
   return __eth_gen(hdr, src, dst);
}

void eth_dissect(loc_t pkt, size_t len)
{
   eth_hdr_t *hdr = (eth_hdr_t*)pkt.addr;

   hdr->type = swap16(hdr->type);

   pkt.linear += sizeof(eth_hdr_t);

   if(hdr->type == ETHER_TYPE_ARP)
      arp_dissect(pkt, len - sizeof(eth_hdr_t));
   else if(hdr->type == ETHER_TYPE_IP)
      ip_dissect(pkt, len - sizeof(eth_hdr_t));
}
