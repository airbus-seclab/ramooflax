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
#ifndef __PCI_E1000_H__
#define __PCI_E1000_H__

#include <types.h>

/*
** Specific Intel 82545/EM pci config space
*/
#define PCI_CFG_DEVICE_i82540EM     0x100e
#define PCI_CFG_DEVICE_i82545EM_C   0x100f
#define PCI_CFG_DEVICE_i82545EM_F   0x1011
#define PCI_CFG_CLASS_i8254x        0x20000
#define PCI_CFG_INT_OFFSET          0x3c

typedef union pci_config_space_e1k_bar
{
   struct
   {
      uint32_t  io:1;     /* 0: mem */
      uint32_t  type:2;   /* 0: 32 bits, 2: 64 bits*/
      uint32_t  pfch:1;   /* 0 non-prefetchable space */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_e1k_bar_t;

/*
** Functions
*/
struct net_info;

void pci_cfg_e1k(struct net_info*);

#endif
