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
#ifndef __DEV_UART_H__
#define __DEV_UART_H__

#include <types.h>
#include <uart.h>
#include <io.h>

#define DEV_UART_BUFF_LEN          (64)
#define DEV_UART_NEED_PROXY        -1

typedef struct uart_device
{
   uint8_t            buffer[DEV_UART_BUFF_LEN];
   uint32_t           index;
#ifndef __UART_PROXY__
   uint16_t           base;
   serial_ier_reg_t   ier;
   serial_iir_reg_t   iir;
   serial_fcr_reg_t   fcr;
   serial_lcr_reg_t   lcr;
   serial_mcr_reg_t   mcr;
   serial_lsr_reg_t   lsr;
   serial_msr_reg_t   msr;
   serial_dla_t       dla;
   uint8_t            scr;
#endif
} uart_t;

/*
** Functions
*/
#ifdef __INIT__
void   dev_uart_init(uart_t*, uint16_t);
#else
int    dev_uart(uart_t*, io_insn_t*);
#endif

#endif
