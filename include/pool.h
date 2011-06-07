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
#ifndef __POOL_H__
#define __POOL_H__

#include <types.h>
#include <cdll.h>

#define POOL_PG_INUSE  1

typedef union pool_page_descriptor_entry
{
   uint16_t flags:12;
   offset_t raw;

} __attribute__((packed)) pool_pg_desc_e_t;

typedef struct pool_page_descriptor
{
   union  pool_page_descriptor_entry page;
   struct pool_page_descriptor       *prev, *next;

} __attribute__((packed)) pool_pg_desc_t;

typedef struct pool_page_list
{
   pool_pg_desc_t *list;
   size_t         nr;

} __attribute__((packed)) pool_pg_list_t;

typedef struct pool_information
{
   offset_t       addr;
   size_t         sz;
   pool_pg_desc_t *all, *used, *free;

} __attribute__((packed)) vmm_pool_t;

/*
** Functions
*/
#ifdef __INIT__
void     pool_init();
#endif

void     pool_push_page(offset_t);
offset_t pool_pop_page();

#endif
