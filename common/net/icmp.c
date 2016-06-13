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
#include <icmp.h>
#include <net.h>
#include <string.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

#ifdef CONFIG_ICMP_DBG
static char* __icmp_type_str(uint8_t type)
{
   switch(type)
   {
   case ICMP_TYPE_ECHO_REPLY:   return "echo-reply";
   case ICMP_TYPE_ECHO_REQUEST: return "echo-request";
   case ICMP_TYPE_DEST_UNREACH: return "dest-unreach";
   case ICMP_TYPE_REDIRECT:     return "redirect";
   case ICMP_TYPE_TIME_EXCEED:  return "time-exceed";
   }

   return "unknown";
}
#endif

static size_t __icmp_gen(icmp_hdr_t *hdr, uint8_t type, uint8_t code, size_t dlen)
{
   size_t len;

   hdr->type = type;
   hdr->code = code;

   len = sizeof(icmp_hdr_t) + dlen;
   icmp_checksum(hdr, len);

   debug(ICMP, "snd ICMP %s rest 0x%x\n", __icmp_type_str(type), hdr->rest);
   return len;
}

static size_t __icmp_echo(icmp_hdr_t *hdr, uint8_t type,
                          uint16_t id, uint16_t seq,
                          void *data, size_t dlen)
{
   loc_t pkt;

   hdr->ping.id  = swap16(id);
   hdr->ping.seq = swap16(seq);

   pkt.addr = hdr;
   pkt.linear += sizeof(icmp_hdr_t);
   memcpy(pkt.addr, data, dlen);

   return __icmp_gen(hdr, type, 0, dlen);
}

size_t icmp_echo_request(icmp_hdr_t *hdr,
                         uint16_t id, uint16_t seq,
                         void *data, size_t dlen)
{
   return __icmp_echo(hdr, ICMP_TYPE_ECHO_REQUEST, id, seq, data, dlen);
}

size_t icmp_echo_reply(icmp_hdr_t *hdr,
                       uint16_t id, uint16_t seq,
                       void *data, size_t dlen)
{
   return __icmp_echo(hdr, ICMP_TYPE_ECHO_REPLY, id, seq, data, dlen);
}

int icmp_dissect(ip_addr_t ip, loc_t pkt, size_t len)
{
   icmp_hdr_t *hdr = (icmp_hdr_t*)pkt.addr;

   debug(ICMP, "rcv ICMP %s\n", __icmp_type_str(hdr->type));

   if(hdr->type == ICMP_TYPE_ECHO_REQUEST)
   {
      net_info_t *net = &info->hrd.dev.net;
      loc_t      rpkt, data;
      size_t     rlen;

      rpkt.linear = net_tx_get_pktbuf();
      if(!rpkt.linear)
      {
         debug(ICMP, "faild to echo-reply (no more pkt)\n");
         return NET_DISSECT_FAIL;
      }

      hdr->ping.id = swap16(hdr->ping.id);
      hdr->ping.seq = swap16(hdr->ping.seq);

      data.linear = pkt.linear + sizeof(icmp_hdr_t);
      rlen = net_gen_pong_pkt(ip, rpkt,
                              hdr->ping.id, hdr->ping.seq,
                              data.addr, len - sizeof(icmp_hdr_t));
      if(!rlen)
      {
#ifdef CONFIG_ICMP_DBG
         char ips[IP_STR_SZ];
         ip_str(ip, ips);
         debug(ICMP, "failed to echo-reply to %s\n", ips);
#endif
         return NET_DISSECT_FAIL;
      }

      net_send_pkt(net, rpkt, rlen);
      return NET_DISSECT_OK;
   }

   return NET_DISSECT_IGN;
}
