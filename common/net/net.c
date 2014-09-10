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
#include <net.h>
#include <string.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

#ifdef __INIT__
void net_init()
{
   net_info_t *net = &info->hrd.dev.net;

   net_init_arch(net);
   arp_init();

   net->ip = 0xac108380; //172.16.131.128
   net->mk = 0xffffff00; //255.255.255.0
   net->gw = 0xac108301; //172.16.131.1
}

offset_t net_mem_init(offset_t area)
{
   net_info_t *net = &info->hrd.dev.net;
   offset_t    off = net_mem_init_arch(&net->arch, area);

   net_rx_init_arch(&net->arch);
   net_tx_init_arch(&net->arch);

   net_rx_on(net);
   net_tx_on(net);

   return off;
}

void net_test_send()
{
   net_info_t *net = &info->hrd.dev.net;
   uint8_t     data[200];
   size_t      len;
   loc_t       pkt;

   pkt.u8 = data;

   //len = net_ping_pkt(net, pkt.addr);
   len = net_udp_pkt(net, pkt.addr);

   while(1)
   {
      net_send_pkt(net, pkt.addr, len);
      io_wait(1000000);
   }
}

void net_test_recv()
{
   net_info_t *net = &info->hrd.dev.net;
   uint8_t     data[200];
   size_t      len;
   loc_t       pkt;

   pkt.u8 = data;

   while(1)
   {
      io_wait(100000);

      /* XXX: use buffer_t instead */
      len = net_recv_pkt(net, pkt.addr, sizeof(data));

      if(!len)
	 continue;

      eth_dissect(pkt, len);
   }
}

void net_test()
{
   //net_test_send();
   net_test_recv();
}

#endif

size_t net_ping_pkt(net_info_t *net, void *addr)
{
   loc_t       pkt;
   eth_hdr_t  *eth;
   ip_hdr_t   *ip;
   icmp_hdr_t *icmp;
   mac_addr_t  dmac;

   mac_rst(&dmac, 0xff);

   pkt.addr = addr;
   eth = (eth_hdr_t*)pkt.addr;
   pkt.linear += sizeof(eth_hdr_t);
   ip = (ip_hdr_t*)pkt.addr;
   pkt.linear += sizeof(ip_hdr_t);
   icmp = (icmp_hdr_t*)pkt.addr;

   return eth_ip(eth, &net->mac, &dmac) + ip_icmp_pkt(ip, icmp_echo_request(icmp));
}

size_t net_udp_pkt(net_info_t *net, void *addr)
{
   loc_t       pkt;
   eth_hdr_t  *eth;
   ip_hdr_t   *ip;
   udp_hdr_t  *udp;
   mac_addr_t  dmac;

   mac_rst(&dmac, 0xff);

   pkt.addr = addr;
   eth = (eth_hdr_t*)pkt.addr;
   pkt.linear += sizeof(eth_hdr_t);
   ip = (ip_hdr_t*)pkt.addr;
   pkt.linear += sizeof(ip_hdr_t);
   udp = (udp_hdr_t*)pkt.addr;

   return eth_ip(eth, &net->mac, &dmac) + ip_udp_pkt(ip, udp_pkt(udp));
}
