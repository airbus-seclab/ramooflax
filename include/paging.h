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
#ifndef __PAGING_H__
#define __PAGING_H__

#include <types.h>
#include <cr.h>
#include <pagemem.h>

/*
** Functions
*/
void pg_map_512G(offset_t, uint32_t);
void pg_map_1G(offset_t, uint32_t);
void pg_map_2M(offset_t, uint32_t);
void pg_map_4K(offset_t, uint32_t);
void pg_map(offset_t, offset_t, uint32_t);

void pg_unmap_512G(offset_t);
void pg_unmap_1G(offset_t);
void pg_unmap_2M(offset_t);
void pg_unmap_4K(offset_t);
void pg_unmap(offset_t, offset_t);

int  __pg_walk(cr3_reg_t*, offset_t, offset_t*, size_t*);
int  pg_walk(offset_t, offset_t*, size_t*);
int  pg_nested_walk(offset_t, offset_t*);
void pg_show(offset_t);

#endif
