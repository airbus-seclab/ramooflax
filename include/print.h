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
#ifndef __PRINT_H__
#define __PRINT_H__

#include <types.h>
#include <config.h>

#define  va_start(v,l)           __builtin_va_start(v,l)
#define  va_end(v)               __builtin_va_end(v)
#define  va_arg(v,l)             __builtin_va_arg(v,l)
typedef  __builtin_va_list       va_list;

#define  panic(format,...)       __panic(__FUNCTION__, format, ## __VA_ARGS__)
void     __panic(const char*, const char*, ...);

size_t   printf(const char*, ... );
size_t   snprintf(char*, size_t, const char*, ...);

size_t   __vprintf(const char*, va_list);
size_t   __vsnprintf(char*, size_t, const char*, va_list);

#define debug_flush()             ({})

#if defined __LOADER__ || defined CONFIG_PRINT_VIDEO
#include <video.h>
#define debug_write(data,size)    video_write(data,size)
#elif defined CONFIG_PRINT_EHCI
#include <ehci.h>
#define debug_write(data,size)    dbgp_write(data,size)
#elif defined CONFIG_PRINT_UART
#include <uart.h>
#define debug_write(data,size)    uart_write(data,size)
#undef debug_flush
#define debug_flush()             uart_flush()
#elif defined CONFIG_PRINT_NET
#include <net.h>
#define debug_write(data,size)    net_write(data,size)
#else
#define debug_write(data,size)    ({})
#endif

#endif
