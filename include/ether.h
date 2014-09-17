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
#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include <types.h>
#include <insn.h>

typedef union mac_address
{
   uint16_t  word[3];
   uint8_t   byte[6];

} __attribute__((packed)) mac_addr_t;

#define MAC_STR_SZ (sizeof(mac_addr_t)*3)

#define ETHER_FRAME_SZ  1500

#define ETHER_TYPE_IP   0x0800
#define ETHER_TYPE_ARP  0x0806

typedef union ethernet_header
{
   struct
   {
      mac_addr_t dst;
      mac_addr_t src;

      union
      {
	 uint16_t   type;
	 uint16_t   len;

      } __attribute__((packed));

   } __attribute__((packed));

   uint8_t raw[14];

} __attribute__((packed)) eth_hdr_t;

/*
** Functions
*/
typedef size_t (*eth_gen_t)(eth_hdr_t*, mac_addr_t*, mac_addr_t*);

size_t eth_ip_pkt(eth_hdr_t*, mac_addr_t*, mac_addr_t*);
size_t eth_arp_pkt(eth_hdr_t*, mac_addr_t*, mac_addr_t*);

int    eth_dissect(loc_t, size_t, buffer_t*);

void   mac_str(mac_addr_t*, char*);
int    mac_cmp(mac_addr_t*, mac_addr_t*);
void   mac_copy(mac_addr_t*, mac_addr_t*);
void   mac_rst(mac_addr_t*, uint8_t);
void   mac_set(mac_addr_t*, char*);

#endif
