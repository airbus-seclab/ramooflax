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
#ifndef __UDP_H__
#define __UDP_H__

#include <types.h>
#include <checksum.h>
#include <insn.h>

typedef union udp_header
{
   struct
   {
      uint16_t   src;
      uint16_t   dst;
      uint16_t   len;
      uint16_t   chk;

   } __attribute__((packed));

   uint8_t raw[8];

} __attribute__((packed)) udp_hdr_t;

typedef union udp_pseudo_header
{
   struct
   {
      uint32_t src;
      uint32_t dst;
      uint8_t  zero;
      uint8_t  proto;
      uint16_t len;

   } __attribute__((packed));

   uint8_t raw[12];

} __attribute__((packed)) udp_phdr_t;

/*
** Functions
*/
size_t udp_pkt(udp_hdr_t*);

#endif
