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
#include <net.h>
#include <string.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

#ifdef __INIT__
static void net_params(mbi_t *mbi)
{
   net_info_t    *net = &info->hrd.dev.net;
   module_t      *mod = (module_t*)((offset_t)mbi->mods_addr + sizeof(module_t));
   mbi_opt_hdl_t  hdl = (mbi_opt_hdl_t)ip_from_str;

   if(!mbi_get_opt(mbi, mod, "ip", hdl, (void*)&net->ip))
      panic("need ip=x.x.x.x on vmm module cmd line\n");

   if(!mbi_get_opt(mbi, mod, "netmask", hdl, (void*)&net->mk))
      panic("need netmask=x.x.x.x on vmm module cmd line\n");

   if(!mbi_get_opt(mbi, mod, "gateway", hdl, (void*)&net->gw))
      panic("need gateway=x.x.x.x on vmm module cmd line\n");

   net->peer.ip = net->gw;
   net->port = 1337;

#ifdef CONFIG_NET_DBG
   {
      char str[IP_STR_SZ];
      ip_str(net->ip, str);
      debug(NET, "ip addr %s\n", str);
      ip_str(net->mk, str);
      debug(PMEM, "netmask %s\n", str);
      ip_str(net->gw, str);
      debug(PMEM, "gateway %s\n", str);
   }
#endif
}
void net_init(mbi_t *mbi)
{
   net_info_t *net = &info->hrd.dev.net;

   net_init_arch(net);
   net_params(mbi);
   arp_init();
}

static void net_arp_gw()
{
   net_info_t *net = &info->hrd.dev.net;
   loc_t      pkt;
   size_t     len;

   pkt.linear = net_tx_get_pktbuf();
   len = net_gen_arp_who_has_pkt(net->gw, pkt);
   net_send_pkt(net, pkt, len);
}

offset_t net_mem_init(offset_t area)
{
   net_info_t *net = &info->hrd.dev.net;
   offset_t    off = net_mem_init_arch(&net->arch, area);

   net_rx_init_arch(&net->arch);
   net_tx_init_arch(&net->arch);

   net_rx_on(net);
   net_tx_on(net);

   net_arp_gw();

   return off;
}


void net_test_recv()
{
   uint8_t data[256];
   size_t  len;

   while(1)
   {
      //io_wait(10000000);
      //io_wait(1000000);

      len = net_read(data, sizeof(data));

      if(!len)
         continue;

      /* debug(NET, "net rcv (%D):\n", len); */

/* #ifdef CONFIG_NET_DBG */
/*       { */
/*       size_t i; */
/*       for(i=0 ; i<len ; i++) */
/*          debug(NET, "%c", data[i]); */
/*       debug(NET,"\n"); */
/*       } */
/* #endif */
   }
}

void net_test()
{
   net_test_recv();
}
#endif

size_t _net_gen_eth_pkt(eth_gen_t builder, mac_addr_t *mac, loc_t pkt)
{
   net_info_t *net;
   eth_hdr_t  *hdr;

   net = &info->hrd.dev.net;
   hdr = (eth_hdr_t*)pkt.addr;

   return builder(hdr, &net->mac, mac);
}

size_t net_gen_eth_ip_pkt(mac_addr_t *mac, loc_t pkt)
{
   return _net_gen_eth_pkt(eth_ip_pkt,mac, pkt);
}

size_t net_gen_eth_arp_pkt(mac_addr_t *mac, loc_t pkt)
{
   return _net_gen_eth_pkt(eth_arp_pkt, mac, pkt);
}

size_t _net_gen_arp_pkt(arp_gen_t builder, mac_addr_t *mac, loc_t pkt,
                        mac_addr_t *hs, mac_addr_t *hd,
                        ip_addr_t s, ip_addr_t d)
{
   arp_hdr_t *hdr;
   loc_t      arp;

   arp.linear = pkt.linear + sizeof(eth_hdr_t);
   hdr = (arp_hdr_t*)arp.addr;

   return net_gen_eth_arp_pkt(mac, pkt) + builder(hdr, hs, hd, s, d);
}

size_t net_gen_arp_who_has_pkt(ip_addr_t ip, loc_t pkt)
{
   net_info_t *net;
   mac_addr_t broad, null;

   net = &info->hrd.dev.net;
   mac_rst(&broad, 0xff);
   mac_rst(&null, 0);

   return _net_gen_arp_pkt(arp_who_has_pkt, &broad, pkt,
                           &net->mac, &null, net->ip, ip);
}

size_t net_gen_arp_is_at_pkt(mac_addr_t *mac, ip_addr_t ip, loc_t pkt)
{
   net_info_t *net = &info->hrd.dev.net;
   return _net_gen_arp_pkt(arp_is_at_pkt, mac, pkt,
                           &net->mac, mac, net->ip, ip);
}

size_t _net_gen_ip_pkt(ip_gen_t builder,
                       ip_addr_t src, ip_addr_t dst,
                       loc_t pkt, size_t dlen)
{
   ip_hdr_t  *hdr;
   loc_t      ip;
   mac_addr_t mac;

   ip.linear = pkt.linear + sizeof(eth_hdr_t);
   hdr = (ip_hdr_t*)ip.addr;

   if(!arp_cache_lookup(dst, &mac))
   {
#ifdef CONFIG_NET_DBG
      {
         char ips[IP_STR_SZ];
         ip_str(dst, ips);
         debug(NET, "can't find %s MAC addr\n", ips);
      }
#endif
      return 0;
   }

   return net_gen_eth_ip_pkt(&mac, pkt) + builder(hdr, src, dst, dlen);
}

size_t _net_gen_icmp_pkt(icmp_gen_t builder,
                         ip_addr_t ip, loc_t pkt,
                         uint16_t id, uint16_t seq,
                         void *data, size_t len)
{
   net_info_t *net = &info->hrd.dev.net;
   icmp_hdr_t *hdr;
   loc_t      icmp;

   icmp.linear = pkt.linear + sizeof(eth_hdr_t) + sizeof(ip_hdr_t);
   hdr = (icmp_hdr_t*)icmp.addr;

   return _net_gen_ip_pkt(ip_icmp_pkt, net->ip, ip, pkt,
                          builder(hdr, id, seq, data, len));
}

size_t net_gen_ping_pkt(ip_addr_t ip, loc_t pkt, uint16_t seq)
{
   return _net_gen_icmp_pkt(icmp_echo_request, ip, pkt,
                            0x1337, seq, (void*)"ram00flax!", 10);
}

size_t net_gen_pong_pkt(ip_addr_t ip, loc_t pkt,
                        uint16_t id, uint16_t seq,
                        void *data, size_t len)
{
   return _net_gen_icmp_pkt(icmp_echo_reply, ip, pkt, id, seq, data, len);
}

size_t net_gen_udp_pkt(ip_addr_t ip, uint16_t port, loc_t pkt, size_t dlen)
{
   net_info_t *net = &info->hrd.dev.net;
   udp_hdr_t  *hdr;
   loc_t       udp;

   udp.linear = pkt.linear + sizeof(eth_hdr_t) + sizeof(ip_hdr_t);
   hdr = (udp_hdr_t*)udp.addr;

   return _net_gen_ip_pkt(ip_udp_pkt, net->ip, ip, pkt,
                          udp_pkt(hdr, net->port, port, dlen));
}

static size_t __net_write(net_info_t *net, buffer_t *buf)
{
   loc_t  pkt, data;
   size_t len;

   pkt.linear  = net_tx_get_pktbuf();
   if(!pkt.linear)
      return 0;

   data.linear = pkt.linear + HDR_FRAME_SZ;
   memcpy(data.addr, buf->data.addr, buf->sz);

   len = net_gen_udp_pkt(net->peer.ip, net->peer.port, pkt, buf->sz);
   if(!len)
      return 0;

   net_send_pkt(net, pkt, len);
   return buf->sz;
}

size_t net_write(uint8_t *data, size_t len)
{
   net_info_t *net = &info->hrd.dev.net;
   buffer_t    buf;
   size_t      times, last, cnt;

   __divrm(len, DATA_FRAME_SZ, times, last);

   buf.data.u8 = data;
   buf.sz = DATA_FRAME_SZ;
   cnt = 0;

   while(times--)
   {
      size_t sz = __net_write(net, &buf);
      cnt += sz;
      buf.data.linear += sz;
   }

   if(last)
   {
      buf.sz = last;
      cnt += __net_write(net, &buf);
   }

   return cnt;
}

static size_t __net_read(net_info_t *net, buffer_t *buf)
{
   static uint8_t frame[RX_BUFF_SZ];
   loc_t          pkt;
   buffer_t       rcv;
   size_t         len;

   pkt.u8 = frame;
   len = net_recv_pkt(net, pkt, sizeof(frame));

   if(!len || eth_dissect(pkt, len, &rcv) != NET_DISSECT_UDP)
      return 0;

   len = min(buf->sz, rcv.sz);
   if(len)
      memcpy(buf->data.addr, rcv.data.addr, len);

   return len;
}

size_t net_read(uint8_t *data, size_t len)
{
   net_info_t *net = &info->hrd.dev.net;
   buffer_t    buf;
   size_t      times, last, cnt;

   __divrm(len, DATA_FRAME_SZ, times, last);

   buf.data.u8 = data;
   buf.sz = DATA_FRAME_SZ;
   cnt = 0;

   while(times--)
   {
      size_t sz = __net_read(net, &buf);

      if(sz < buf.sz)
         return cnt;

      cnt += sz;
      buf.data.linear += sz;
   }

   if(last)
   {
      buf.sz = last;
      cnt += __net_read(net, &buf);
   }

   return cnt;
}

