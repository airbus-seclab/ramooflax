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
#include <ip.h>
#include <icmp.h>
#include <string.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

void ip_str(ip_addr_t ip, char *str)
{
   raw32_t i = { .raw = ip };

   snprintf(str, IP_STR_SZ, "%d.%d.%d.%d"
	    ,i._whigh.bhigh
	    ,i._whigh.blow
	    ,i._wlow.bhigh
	    ,i._wlow.blow);
}

size_t ip_icmp_pkt(ip_hdr_t *hdr, size_t len)
{
   size_t plen = sizeof(ip_hdr_t) + len;

   memset(hdr, 0, sizeof(ip_hdr_t));

   hdr->ver = 4;
   hdr->ihl = sizeof(ip_hdr_t)/sizeof(uint32_t);
   hdr->ttl = 0xff;
   hdr->proto = IP_PROTO_ICMP;

   hdr->frag.raw = swap16(hdr->frag.raw);
   hdr->id = swap16(1);
   hdr->len = swap16(plen);
   hdr->src = swap32(0x7f000001);
   hdr->dst = swap32(0xffffffff);

   ip_checksum(hdr);

   debug(IP, "ip icmp pkt len %d (hdr %d)\n", plen, sizeof(ip_hdr_t));
   return plen;
}

size_t ip_udp_pkt(ip_hdr_t *hdr, size_t len)
{
   loc_t      loc;
   udp_phdr_t *pudp;
   udp_hdr_t  *udp;
   ip_addr_t  ipsrc, ipdst;
   size_t     plen;

   ipsrc = 0x7f000001;
   ipdst = 0x0a5600e1; //10.86.0.225

   loc.addr = hdr;
   loc.linear += sizeof(ip_hdr_t);
   udp = (udp_hdr_t*)loc.addr;

   loc.linear -= sizeof(udp_phdr_t);
   pudp = (udp_phdr_t*)loc.addr;

   pudp->src = swap32(ipsrc);
   pudp->dst = swap32(ipdst);
   pudp->zero = 0;
   pudp->proto = IP_PROTO_UDP;
   pudp->len = swap16(len);

   plen = sizeof(udp_phdr_t) + len;
   if(plen % 2)
   {
      loc.linear += plen;
      *loc.u8 = 0;
      plen++;
   }

   udp->chk = rfc1071_checksum((uint16_t*)pudp, plen);
   if(!udp->chk)
      udp->chk = ~udp->chk;

   debug(IP, "ip udp chk 0x%x\n", udp->chk);

   plen = sizeof(ip_hdr_t) + len;

   memset(hdr, 0, sizeof(ip_hdr_t));

   hdr->ver = 4;
   hdr->ihl = sizeof(ip_hdr_t)/sizeof(uint32_t);
   hdr->ttl = 0xff;
   hdr->proto = IP_PROTO_UDP;

   hdr->frag.raw = swap16(hdr->frag.raw);
   hdr->id = swap16(1);
   hdr->len = swap16(plen);
   hdr->src = swap32(ipsrc);
   hdr->dst = swap32(ipdst);

   ip_checksum(hdr);

   debug(IP, "ip udp pkt len %d (hdr %d)\n", plen, sizeof(ip_hdr_t));
   return plen;
}

void ip_dissect(loc_t pkt, size_t len)
{
   ip_hdr_t *hdr = (ip_hdr_t*)pkt.addr;

   debug(IP, "rcv ip pkt %d\n", len);
}
