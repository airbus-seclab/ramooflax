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
#ifndef __DEV_H__
#define __DEV_H__

#include <config.h>
#include <types.h>
#include <smap.h>
#include <ppg.h>
#include <io.h>
#include <mbi.h>

#ifdef CONFIG_HAS_EHCI
#include <ehci.h>
#endif
#ifdef CONFIG_HAS_NET
#include <net.h>
#endif

typedef struct hardware_device_data
{
#ifdef CONFIG_HAS_EHCI
   dbgp_info_t dbgp;
#endif
#ifdef CONFIG_HAS_NET
   net_info_t  net;
#endif

} __attribute__((packed)) hrdw_dev_t;

typedef struct hardware_memory_data
{
   offset_t    top;  /* max mapped virtual address */
   ppg_info_t  ppg;  /* physical page descriptors  */

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
void  dev_init(mbi_t *mbi);
#else
int   dev_access();
void  dev_a20_set(uint8_t);
#endif

#endif
