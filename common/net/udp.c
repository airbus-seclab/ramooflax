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
#include <udp.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

size_t udp_pkt(udp_hdr_t *hdr, uint16_t sport, uint16_t dport, size_t dlen)
{
   size_t len = sizeof(udp_hdr_t) + dlen;

   hdr->src = swap16(sport);
   hdr->dst = swap16(dport);
   hdr->len = swap16(len);
   hdr->chk = 0;

   debug(UDP, "snd UDP sport %d dport %d len %D\n", sport, dport, len);

#ifdef CONFIG_UDP_DBG
   {
      size_t i;
      loc_t data;
      data.addr = hdr;
      data.linear += sizeof(udp_hdr_t);
      printf("snd UDP data (%D): ", dlen);
      for(i=0 ; i<dlen ; i++)
         printf("%c", data.u8[i]);
      printf("\n");
   }
#endif
   return len;
}

int udp_dissect(loc_t pkt, size_t len, buffer_t *rcv)
{
   net_info_t *net = &info->hrd.dev.net;
   udp_hdr_t  *hdr = (udp_hdr_t*)pkt.addr;

   hdr->src = swap16(hdr->src);
   hdr->dst = swap16(hdr->dst);
   hdr->len = swap16(hdr->len);

   debug(UDP, "rcv UDP src %d dst %d hdr len %d pkt len %D\n"
         ,hdr->src, hdr->dst, hdr->len, len);

   rcv->data.linear = pkt.linear + sizeof(udp_hdr_t);
   rcv->sz = min(len, hdr->len) - sizeof(udp_hdr_t);

#ifdef CONFIG_UDP_DBG
   {
      size_t i;
      printf("rcv UDP data (%d): ", rcv->sz);
      for(i=0 ; i<rcv->sz ; i++)
         printf("%c", rcv->data.u8[i]);
      printf("\n");
   }
#endif

   if(hdr->dst != net->port)
      return NET_DISSECT_IGN;

   /* XXX */
   if(net->peer.port != hdr->src)
      net->peer.port = hdr->src;

   return NET_DISSECT_UDP;
}
