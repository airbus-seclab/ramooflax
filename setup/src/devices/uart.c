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
#include <dev_uart.h>

void dev_uart_init(uart_t *uart, uint16_t base)
{
   uart->index = 0;
   uart->base  = base;

   uart->iir.no_int_pending = 1;
   uart->iir.info = SERIAL_IIR_INFO_THR_EMPTY;

   uart->ier.raw = 0;
   uart->lcr.raw = 0;
   uart->mcr.raw = 0;
   uart->lsr.raw = 0x60;
   uart->msr.raw = 0;
   uart->dla.raw = 0;
   uart->scr     = 0;
}
