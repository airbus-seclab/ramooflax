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
#include <dev.h>
#include <uart.h>
#include <debug.h>
#include <info_data.h>

#ifdef CONFIG_HAS_EHCI
#include <ehci.h>
#endif
#ifdef CONFIG_HAS_NET
#include <net.h>
#endif

extern info_data_t *info;

void dev_init(mbi_t *mbi)
{
#ifdef CONFIG_HAS_UART
   uart_init();
#endif

#ifdef CONFIG_HAS_EHCI
   ehci_init();
#endif

#ifdef CONFIG_HAS_NET
   net_init(mbi);
#endif
}
