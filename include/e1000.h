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
#ifndef __E1000_H__
#define __E1000_H__

#include <types.h>
#include <pci_e1000.h>

/*
** Intel 82545/EM driver
*/

/*
** Convenient structure
*/
typedef struct e1000_info
{
   pci_cfg_val_t  pci;

} __attribute__((packed)) e1000_info_t;

/*
** Functions
*/
#ifdef __INIT__
void e1000_init();
#endif

size_t e1000_write(uint8_t*, size_t);
size_t e1000_read(uint8_t*, size_t);


#endif
