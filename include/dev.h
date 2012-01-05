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
#ifndef __DEV_H__
#define __DEV_H__

#include <config.h>
#include <types.h>
#include <smap.h>
#include <ehci.h>
#include <io.h>

typedef struct hardware_device_data
{
#ifdef CONFIG_HAS_EHCI
   dbgp_info_t dbgp;
#endif

} __attribute__((packed)) hrdw_dev_t;

typedef struct hardware_memory_data
{
   offset_t top;

} __attribute__((packed)) hrdw_mem_t;

typedef struct hardware_information_data
{
   hrdw_dev_t dev;
   hrdw_mem_t mem;

} __attribute__((packed)) hrdw_t;

/*
** Funtions
*/
#ifdef __INIT__
void  dev_init();
#else
int   dev_access();
void  dev_a20_set(uint8_t);
#endif

#endif
