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

#ifdef CONFIG_IP_DBG
static char* __ip_proto_str(uint8_t proto)
{
   switch(proto)
   {
   case IP_PROTO_ICMP:   return "icmp";
   case IP_PROTO_IGMP:   return "igmp";
   case IP_PROTO_GGP:    return "ggp";
   case IP_PROTO_IPINIP: return "ipinip";
   case IP_PROTO_TCP:    return "tcp";
   case IP_PROTO_EGP:    return "egp";
   case IP_PROTO_IGP:    return "igp";
   case IP_PROTO_CHAOS:  return "chaos";
   case IP_PROTO_UDP:    return "udp";
   case IP_PROTO_RDP:    return "rdp";
   case IP_PROTO_IP6:    return "ip6";
   case IP_PROTO_GRE:    return "gre";
   case IP_PROTO_ICMP6:  return "icmp6";
   case IP_PROTO_CFTP:   return "cftp";
   case IP_PROTO_VISA:   return "visa";
   case IP_PROTO_TTP:    return "ttp";
   case IP_PROTO_OSPF:   return "ospf";
   case IP_PROTO_L2TP:   return "l2tp";
   case IP_PROTO_PTP:    return "ptp";
   case IP_PROTO_SCTP:   return "sctp";
   }
   return "unknown";
}
#endif

void ip_str(ip_addr_t ip, char *str)
{
   raw32_t i = { .raw = ip };

   snprintf(str, IP_STR_SZ, "%d.%d.%d.%d"
	    ,i._whigh.bhigh
	    ,i._whigh.blow
	    ,i._wlow.bhigh
	    ,i._wlow.blow);
}

int ip_from_str(char *str, ip_addr_t *ip)
{
   size_t    i, n, m, max;
   uint64_t  x;

   max = min(strlen(str), 15);
   i = n = 0; m = 3;
   *ip = 0;

   while(m != 0)
   {
      while(i < max && str[i] != '.') i++;

      if(i == max || !dec_to_uint64((uint8_t*)&str[n], i - n, &x) || x > 255)
	 return 0;

      *ip |= x<<(m--*8);
      n = ++i;
   }

   if(n == max || !dec_to_uint64((uint8_t*)&str[n], max - n, &x) || x > 255)
      return 0;

   *ip |= x;
   return 1;
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
      debug(IP, "snd IP src %s dst %s len %D\n", ips, ipd, len);
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

   udp->chk = swap16(rfc1071_checksum((uint16_t*)psd, len));
   if(!udp->chk)
      udp->chk = ~udp->chk;

   /* debug(IP, "snd UDP chk 0x%x\n", udp->chk); */
   return __ip_gen(hdr, src, dst, IP_PROTO_UDP, dlen);
}

size_t ip_icmp_pkt(ip_hdr_t *hdr, ip_addr_t src, ip_addr_t dst, size_t dlen)
{
   return __ip_gen(hdr, src, dst, IP_PROTO_ICMP, dlen);
}

int ip_dissect(loc_t pkt, size_t len, buffer_t *rcv)
{
   ip_hdr_t *hdr = (ip_hdr_t*)pkt.addr;

   if(hdr->ver != 4)
      return NET_DISSECT_FAIL;

   hdr->frag.raw = swap16(hdr->frag.raw);
   hdr->id = swap16(hdr->id);
   hdr->len = swap16(hdr->len);
   hdr->src = swap32(hdr->src);
   hdr->dst = swap32(hdr->dst);

#ifdef CONFIG_IP_DBG
   {
      char ips[IP_STR_SZ];
      char ipd[IP_STR_SZ];
      ip_str(hdr->src, ips);
      ip_str(hdr->dst, ipd);
      debug(IP, "rcv IP %s src %s dst %s len %d id %d off %d mf %d df %d\n"
	    ,__ip_proto_str(hdr->proto), ips, ipd, hdr->len
	    ,hdr->id, hdr->frag.off, hdr->frag.mf, hdr->frag.df);
   }
#endif

   pkt.linear += sizeof(ip_hdr_t);

   if(hdr->proto == IP_PROTO_ICMP)
      return icmp_dissect(hdr->src, pkt, len - sizeof(ip_hdr_t));
   else if(hdr->proto == IP_PROTO_UDP)
      return udp_dissect(pkt, len - sizeof(ip_hdr_t), rcv);

   return NET_DISSECT_IGN;
}
