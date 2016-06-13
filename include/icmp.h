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
#ifndef __ICMP_H__
#define __ICMP_H__

#include <types.h>
#include <checksum.h>
#include <insn.h>
#include <ip.h>

#define ICMP_TYPE_ECHO_REPLY      0
#define ICMP_TYPE_ECHO_REQUEST    8
#define ICMP_TYPE_DEST_UNREACH    3
#define ICMP_TYPE_REDIRECT        5
#define ICMP_TYPE_TIME_EXCEED    11

#define ICMP_CODE_UNREACH_NET    0
#define ICMP_CODE_UNREACH_HOST   1
#define ICMP_CODE_UNREACH_PROT   2
#define ICMP_CODE_UNREACH_PORT   3
#define ICMP_CODE_UNREACH_FRAG   4
#define ICMP_CODE_UNREACH_ROUTE  5
#define ICMP_CODE_UNREACH_UNET   6
#define ICMP_CODE_UNREACH_UHOST  7

#define ICMP_CODE_TIME_EXCEED_TTL  0
#define ICMP_CODE_TIME_EXCEED_FRG  1

typedef union icmp_header
{
   struct
   {
      uint8_t  type;
      uint8_t  code;
      uint16_t chk;

      union
      {
         struct
         {
            uint16_t  id;
            uint16_t  seq;
         } ping;

         uint32_t rest;

      } __attribute__((packed));

   } __attribute__((packed));

   uint8_t raw[8];

} __attribute__((packed)) icmp_hdr_t;

/*
** Functions
*/
#define icmp_checksum(_hdr_, _ln_)                                      \
   ({                                                                   \
      (_hdr_)->chk = 0;                                                 \
      (_hdr_)->chk = swap16(rfc1071_checksum((uint16_t*)(_hdr_), (_ln_))); \
   })

typedef size_t (*icmp_gen_t)(icmp_hdr_t*, uint16_t, uint16_t, void*, size_t);

size_t icmp_echo_request(icmp_hdr_t*, uint16_t, uint16_t, void*, size_t);
size_t icmp_echo_reply(icmp_hdr_t*, uint16_t, uint16_t, void*, size_t);
int    icmp_dissect(ip_addr_t, loc_t, size_t);

#endif
