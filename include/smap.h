/*
** Copyright (C) 2011 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#ifndef __SMAP_H__
#define __SMAP_H__

#include <types.h>

#define SMAP_TYPE_AVL   1   /* memory, available to OS */
#define SMAP_TYPE_RSV   2   /* reserved, not available (rom, mem map dev) */
#define SMAP_TYPE_ACPI  3   /* ACPI Reclaim Memory */
#define SMAP_TYPE_NVS   4   /* ACPI NVS Memory */

typedef struct smap_entry
{
   uint64_t    base_addr;
   uint64_t    len;
   uint32_t    type;

} __attribute__((packed)) smap_e_t;

typedef struct bios_smap
{
   size_t      nr;        /* number of entries */

   union
   {
      smap_e_t *entries;  /* smap entries */
      uint8_t  *raw;

   } __attribute__((packed));

} __attribute__((packed)) smap_t;

#ifdef __INIT__
#include <mbi.h>
void   smap_parse(mbi_t*, smap_t*, offset_t*, offset_t*);
void   smap_init(mbi_t*, smap_t*, offset_t);
#endif

#endif
