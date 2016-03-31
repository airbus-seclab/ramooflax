/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#ifndef __PHYSICAL_PAGES_H__
#define __PHYSICAL_PAGES_H__

#include <types.h>
#include <pagemem.h>
#include <smap.h>

typedef struct physical_page_descriptor
{
   offset_t  addr;
   size_t    count;

   struct
   {
      uint64_t   vmm:1;   /* vmm owned */

   } __attribute__((packed));

   struct physical_page_descriptor  *prev;
   struct physical_page_descriptor  *next;

} __attribute__((packed)) ppg_dsc_t;

typedef struct physical_page_list
{
   ppg_dsc_t *list;
   size_t     nr;

} __attribute__((packed)) ppg_list_t;

/*
** Physical Memory Page descriptors
**
** descriptor refers to contigous pages
** from 0 to maximum available page
** to linearly acces them even if all pages
** are not available
*/
typedef struct physical_memory_page_info
{
   ppg_dsc_t   *dsc;   /* all ppg desc */
   size_t       nr;

   ppg_list_t   vmm;   /* vmm owned ppg desc */
   ppg_list_t   vm;    /* vm  owned ppg desc */

} __attribute__((packed)) ppg_info_t;

/*
** Functions
*/
#ifdef __INIT__
void  ppg_desc_init();
void  ppg_desc_set_region_type(smap_e_t*);
void  debug_ppg();
#endif

ppg_dsc_t* ppg_get_desc(offset_t);

#endif
