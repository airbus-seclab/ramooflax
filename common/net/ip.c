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

static size_t __ip_gen(ip_hdr_t *hdr,
		       ip_addr_t src, ip_addr_t dst,
		       uint8_t proto, size_t dlen)
{
   size_t len = sizeof(ip_hdr_t) + dlen;

   hdr->ver = 4;
   hdr->ihl = sizeof(ip_hdr_t)/sizeof(uint32_t);
   hdr->ecn = hdr-> dscp = 0;
   hdr->ttl = 0xff;
   hdr->proto = proto;

   hdr->frag.raw = 0;
   hdr->id = swap16(1);
   hdr->len = swap16(len);
   hdr->src = swap32(src);
   hdr->dst = swap32(dst);

   ip_checksum(hdr);

#ifdef CONFIG_IP_DBG
   {
      char ips[IP_STR_SZ];
      char ipd[IP_STR_SZ];
      ip_str(src, ips);
      ip_str(dst, ipd);
      debug(IP, "snd IP src %s dst %s len %d\n", ips, ipd, len);
   }
#endif

   return len;
}

size_t ip_udp_pkt(ip_hdr_t *hdr, ip_addr_t src, ip_addr_t dst, size_t dlen)
{
   udp_phdr_t *psd;
   udp_hdr_t  *udp;
   loc_t       pkt;
   size_t      len;

   pkt.addr = hdr;
   pkt.linear += sizeof(ip_hdr_t);
   udp = (udp_hdr_t*)pkt.addr;

   pkt.linear -= sizeof(udp_phdr_t);
   psd = (udp_phdr_t*)pkt.addr;

   psd->src = swap32(src);
   psd->dst = swap32(dst);
   psd->zero = 0;
   psd->proto = IP_PROTO_UDP;
   psd->len = swap16(dlen);

   /* XXX: off-by-one
   ** pkt buffer are always greater than
   ** maximum frame sz, so it's safe to
   ** align on 2 bytes
   */
   len = sizeof(udp_phdr_t) + dlen;
   if(len % 2)
   {
      pkt.linear += len;
      *pkt.u8 = 0;
      len++;
   }

   udp->chk = rfc1071_checksum((uint16_t*)psd, len);
   if(!udp->chk)
      udp->chk = ~udp->chk;

   /* debug(IP, "snd UDP chk 0x%x\n", udp->chk); */
   return __ip_gen(hdr, src, dst, IP_PROTO_UDP, dlen);
}

size_t ip_icmp_pkt(ip_hdr_t *hdr, ip_addr_t src, ip_addr_t dst, size_t dlen)
{
   return __ip_gen(hdr, src, dst, IP_PROTO_ICMP, dlen);
}

void ip_dissect(loc_t pkt, size_t len)
{
   ip_hdr_t *hdr = (ip_hdr_t*)pkt.addr;

   if(hdr->ver != 4)
      return;

   hdr->frag.raw = swap16(hdr->frag.raw);
   hdr->id = swap16(hdr->id);
   hdr->len = swap16(hdr->len);
   hdr->src = swap32(hdr->src);
   hdr->dst = swap32(hdr->src);

   pkt.linear += sizeof(ip_hdr_t);

   if(hdr->proto == IP_PROTO_ICMP)
      icmp_dissect(hdr->src, pkt, len - sizeof(ip_hdr_t));
   else if(hdr->proto == IP_PROTO_UDP)
      udp_dissect(pkt, len - sizeof(ip_hdr_t));
}
