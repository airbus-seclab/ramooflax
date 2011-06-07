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
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static inline void __dev_uart_flush(uart_t *uart)
{
   debug_write(uart->buffer, uart->index);
   uart->index  = 0;
}

static inline int __dev_uart_tx(uart_t *uart, io_insn_t *io)
{
   io_size_t sz;

   do
   {
      sz.available = DEV_UART_BUFF_LEN - uart->index;

      if(!dev_io_insn(io, &uart->buffer[uart->index], &sz))
	 return 0;

      uart->index += sz.done;

      if(uart->index >= DEV_UART_BUFF_LEN)
      {
	 uart->index = DEV_UART_BUFF_LEN;
	 __dev_uart_flush(uart);
      }

   } while(sz.miss);

   return 1;
}

#ifndef __UART_PROXY__
/*
** XXX
*/
static inline int __dev_uart_dla(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };

   if(io->port == SERIAL_DLA_LSB(uart->base))
      return dev_io_insn(io, &uart->dla.lsb, &sz);
   else
      return dev_io_insn(io, &uart->dla.msb, &sz);
}

/*
** XXX
*/
static inline int __dev_uart_lsr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   return dev_io_insn(io, &uart->lsr.raw, &sz);
}

/*
** XXX
*/
static inline int __dev_uart_ier(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   return dev_io_insn(io, &uart->ier.raw, &sz);
}

/*
** XXX
*/
static inline int __dev_uart_iir(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   return dev_io_insn(io, &uart->iir.raw, &sz);
}

/*
** XXX
*/
static inline int __dev_uart_fcr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   return dev_io_insn(io, &uart->fcr.raw, &sz);
}

/*
** XXX
*/
static inline int __dev_uart_lcr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   return dev_io_insn(io, &uart->lcr.raw, &sz);
}

/*
** XXX
*/
static inline int __dev_uart_mcr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   return dev_io_insn(io, &uart->mcr.raw, &sz);
}

/*
** XXX
*/
static inline int __dev_uart_msr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   return dev_io_insn(io, &uart->msr.raw, &sz);
}

/*
** XXX
*/
static inline int __dev_uart_scr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   return dev_io_insn(io, &uart->scr, &sz);
}

/*
** XXX
*/
static inline int __dev_uart_rx(/*uart_t *uart,*/ io_insn_t *io)
{
   uint8_t    x  = 0;
   io_size_t  sz = { .available = 1 };
   return dev_io_insn(io, &x, &sz);
}

static inline int __dev_uart_xfr(uart_t *uart, io_insn_t *io)
{
   if(io->d)
      return __dev_uart_rx(/*uart,*/ io);

   return __dev_uart_tx(uart, io);
}

int dev_uart(uart_t *uart, io_insn_t *io)
{
   if(io->port <= SERIAL_IER(uart->base))
   {
      if(uart->lcr.dla)
	 return __dev_uart_dla(uart, io);

      if(io->port == SERIAL_TXRX(uart->base))
	 return __dev_uart_xfr(uart, io);

      return __dev_uart_ier(uart, io);
   }

   if(io->port == SERIAL_IIR(uart->base))
   {
      if(io->d)
	 return __dev_uart_iir(uart, io);

      return __dev_uart_fcr(uart, io);
   }

   if(io->port == SERIAL_LCR(uart->base))
      return __dev_uart_lcr(uart, io);

   if(io->port == SERIAL_MCR(uart->base))
      return __dev_uart_mcr(uart, io);

   if(io->port == SERIAL_LSR(uart->base) && io->d)
      return __dev_uart_lsr(uart, io);

   if(io->port == SERIAL_MSR(uart->base) && io->d)
      return __dev_uart_msr(uart, io);

   if(io->port == SERIAL_SCR(uart->base))
      return __dev_uart_scr(uart, io);

   debug(DEV_UART, "unsupported uart operation !\n");
   return 0;
}
#else
int dev_uart(uart_t *uart, io_insn_t *io)
{
   if(io->port != SERIAL_TXRX(SERIAL_COM1) || io->d)
      return DEV_UART_NEED_PROXY;

   return __dev_uart_tx(uart, io);
}
#endif

