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
#ifndef __NET_H__
#define __NET_H__

#include <config.h>
#include <types.h>
#include <pci.h>
#include <ether.h>
#include <arp.h>
#include <ip.h>
#include <icmp.h>
#include <udp.h>

#ifdef CONFIG_HAS_E1000
#include <e1000.h>
#endif

/* XXX: fix size to that of subnet mask */
#define ARP_CACHE_SZ  256

/*
** Generic Network Interface information
*/
typedef struct net_info
{
   pci_cfg_val_t  pci;
   uint8_t        irq;
   arp_tbl_t      arp;
   mac_addr_t     mac;
   ip_addr_t      ip;
   ip_addr_t      mk;
   ip_addr_t      gw;

#ifdef CONFIG_HAS_E1000
   e1k_info_t     arch;
#endif

} __attribute__((packed)) net_info_t;

/*
** Functions
*/
#ifdef __INIT__
void     net_init();
offset_t net_mem_init(offset_t);
void     net_test();

#ifdef CONFIG_HAS_E1000
#define net_init_arch(n)          e1k_init(n)
#define net_mem_size_arch()       e1k_mem_size()
#define net_mem_init_arch(n,o)    e1k_mem_init(n,o)
#define net_rx_init_arch(n)       e1k_rx_init(n)
#define net_tx_init_arch(n)       e1k_tx_init(n)
#endif

#endif

size_t net_ping_pkt(net_info_t*, void*);
size_t net_udp_pkt(net_info_t*, void*);

#ifdef CONFIG_HAS_E1000
#define net_rx_on(n)               e1k_rx_on(n)
#define net_tx_on(n)               e1k_tx_on(n)
#define net_send_pkt(n,p,l)        e1k_send_pkt(n,p,l)
#define net_recv_pkt(n,p,l)        e1k_recv_pkt(n,p,l)
#endif

#endif
