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
#include <arp.h>
#include <string.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

#ifdef __INIT__
void arp_init()
{
   arp_tbl_t *arp = &info->hrd.dev.net.arp;
   memset((void*)arp->cache, 0, ARP_CACHE_SZ*sizeof(arp_cache_t));
   arp->tail = 0;
}
#endif

static size_t __arp_op(arp_hdr_t *hdr, uint16_t op,
		       mac_addr_t *hwsrc, mac_addr_t *hwdst,
		       ip_addr_t src, ip_addr_t dst)
{
   hdr->hwtype = swap16(ARP_TYPE_ETH);
   hdr->proto = swap16(ETHER_TYPE_IP);
   hdr->hlen = sizeof(mac_addr_t);
   hdr->plen = sizeof(ip_addr_t);
   hdr->op = swap16(op);

   mac_copy(&hdr->hwsrc, hwsrc);
   mac_copy(&hdr->hwdst, hwdst);

   hdr->psrc = swap32(src);
   hdr->pdst = swap32(dst);

   return sizeof(arp_hdr_t);
}

size_t arp_who_has_pkt(arp_hdr_t *hdr,
		       mac_addr_t *hwsrc, mac_addr_t *hwdst,
		       ip_addr_t src, ip_addr_t dst)
{
   return __arp_op(hdr, ARP_OP_REQUEST, hwsrc, hwdst, src, dst);
}

size_t arp_is_at_pkt(arp_hdr_t *hdr,
		     mac_addr_t *hwsrc, mac_addr_t *hwdst,
		     ip_addr_t src, ip_addr_t dst)
{
   return __arp_op(hdr, ARP_OP_REPLY, hwsrc, hwdst, src, dst);
}

static void arp_cache_update(ip_addr_t ip, mac_addr_t *mac)
{
   arp_tbl_t *arp = &info->hrd.dev.net.arp;
   size_t     i, end;

#ifdef CONFIG_ARP_DBG
   {
      char ips[IP_STR_SZ];
      char macs[MAC_STR_SZ];
      ip_str(ip, ips);
      mac_str(mac, macs);
      debug(ARP, "arp cache update %s %s\n", ips, macs);
   }
#endif

   end = (!arp->cache[arp->tail].ip) ? arp->tail : ARP_CACHE_SZ;

   for(i=0 ; i<end ; i++)
      if(arp->cache[i].ip == ip)
      {
	 if(!mac_cmp(&arp->cache[i].mac, mac))
	    mac_copy(&arp->cache[i].mac, mac);

	 return;
      }

   arp->cache[arp->tail].ip = ip;
   mac_copy(&arp->cache[arp->tail].mac, mac);
   arp->tail = (arp->tail+1)%ARP_CACHE_SZ;
}

int arp_cache_lookup(ip_addr_t ip, mac_addr_t *mac)
{
   arp_tbl_t *arp = &info->hrd.dev.net.arp;
   size_t     i, end;

   end = (!arp->cache[arp->tail].ip) ? arp->tail : ARP_CACHE_SZ;

   for(i=0 ; i<end ; i++)
      if(arp->cache[i].ip == ip)
      {
	 mac_copy(mac, &arp->cache[i].mac);
	 return 1;
      }

   return 0;
}

void __arp_dissect_who_has(arp_hdr_t *hdr)
{
   net_info_t *net = &info->hrd.dev.net;
   loc_t       pkt;
   size_t      len;

#ifdef CONFIG_ARP_DBG
   {
      char ips[IP_STR_SZ];
      char ipd[IP_STR_SZ];
      ip_str(hdr->pdst, ipd);
      ip_str(hdr->psrc, ips);
      debug(ARP, "arp who_has %s say %s\n", ipd, ips);
   }
#endif

   arp_cache_update(hdr->psrc, &hdr->hwsrc);

   if(hdr->pdst != net->ip)
      return;

   pkt.linear = net_tx_get_pktbuf();
   if(!pkt.linear)
   {
      debug(ARP, "failed to arp-is-at (no more pkt)\n");
      return;
   }

   len = net_gen_arp_is_at_pkt(&hdr->hwsrc, hdr->psrc, pkt);
   net_send_pkt(net, pkt, len);

#ifdef CONFIG_ARP_DBG
   {
      char ips[IP_STR_SZ];
      char macs[MAC_STR_SZ];
      ip_str(net->ip, ips);
      mac_str(&net->mac, macs);
      debug(ARP, "arp is_at %s say %s\n", macs, ips);
   }
#endif
}

void __arp_dissect_is_at(arp_hdr_t *hdr)
{
#ifdef CONFIG_ARP_DBG
   {
      char ips[IP_STR_SZ];
      char macs[MAC_STR_SZ];
      ip_str(hdr->psrc, ips);
      mac_str(&hdr->hwsrc, macs);
      debug(ARP, "arp %s is_at %s\n", ips, macs);
   }
#endif

   arp_cache_update(hdr->psrc, &hdr->hwsrc);
}

void arp_dissect(loc_t pkt, size_t __unused__ len)
{
   arp_hdr_t *hdr = (arp_hdr_t*)pkt.addr;

   hdr->op = swap16(hdr->op);
   hdr->hwtype = swap16(hdr->hwtype);
   hdr->proto = swap16(hdr->proto);

   if(hdr->hwtype != ARP_TYPE_ETH   ||
      hdr->proto  != ETHER_TYPE_IP)
      return;

   hdr->psrc = swap32(hdr->psrc);
   hdr->pdst = swap32(hdr->pdst);

   if(hdr->op == ARP_OP_REQUEST)
      __arp_dissect_who_has(hdr);
   else if(hdr->op == ARP_OP_REPLY)
      __arp_dissect_is_at(hdr);
}
