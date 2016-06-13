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
#ifndef __CTRL_IO_H__
#define __CTRL_IO_H__

#include <config.h>

#ifdef CONFIG_REMOTE_EHCI
#include <ehci.h>
#define ctrl_io_read(data,size)     dbgp_read(data,size)
#define ctrl_io_write(data,size)    dbgp_write(data,size)
#elif defined CONFIG_REMOTE_UART
#include <uart.h>
#define ctrl_io_read(data,size)     uart_read(data,size)
#define ctrl_io_write(data,size)    uart_write(data,size)
#elif defined CONFIG_REMOTE_NET
#include <net.h>
#define ctrl_io_read(data,size)     net_read(data,size)
#define ctrl_io_write(data,size)    net_write(data,size)
#else
#define ctrl_io_read(data,size)     ({0;})
#define ctrl_io_write(data,size)    ({})
#endif

#endif
