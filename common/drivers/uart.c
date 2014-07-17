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
#include <uart.h>
#include <info_data.h>

extern info_data_t *info;

#ifdef __INIT__
static void __uart_flush_recv(uint16_t port)
{
   while(__uart_can_recv(port))
      __uart_recv_char(port);
}

static void __uart_fifo_init(uint16_t port)
{
   serial_fcr_reg_t  fcr;
   serial_efr_reg_t  efr;

   uart_enable_efr_registers(port);
   efr.raw = 0;
   efr.ctl = 1;
   out(efr.raw, SERIAL_EFR(port));

   fcr.enable = 0;
   fcr.dma = 0;
   out(fcr.raw, SERIAL_FCR(port));

   out(0, SERIAL_LCR(port));

   efr.raw = 0;
   efr.a_rts = 1;
   efr.a_cts = 1;
   out(efr.raw, SERIAL_EFR(port));
}

static void __uart_common_init(uint16_t port)
{
   serial_ier_reg_t  ier;
   serial_lcr_reg_t  lcr;
   /* serial_mcr_reg_t  mcr; */

   /* set baud rate to 115200 */
   uart_enable_dla_registers(port);
   uart_set_lsb_dla_rate(port, 0x01);
   uart_set_msb_dla_rate(port, 0x00);

   /* 8 bits, 1 stop bit, no parity */
   lcr.raw = 0;
   lcr.word_len = 3;
   out(lcr.raw, SERIAL_LCR(port));

   /*
   ** IRQ activation bug (reap from Linux)
   ** for newer uarts
   */
   /* mcr.raw = in(SERIAL_MCR(port)); */
   /* mcr.aux2 = 1; */
   /* out(mcr.raw, SERIAL_MCR(port)); */

   /* trigger interrupt on byte received */
   ier.raw = 0;
   /* ier.recv = 1; */
   out(ier.raw, SERIAL_IER(port));

   /* flush input */
   __uart_flush_recv(port);
}

void uart_init()
{
   __uart_fifo_init(SERIAL_COM1);
   __uart_common_init(SERIAL_COM1);
}
#endif

static inline void __uart_read(uint16_t port, buffer_t *buf, size_t max)
{
   while(__uart_can_recv(port) && buf->sz < max)
   {
      buf->data.u8[buf->sz] = __uart_recv_char(port);
      buf->sz++;
   }
}

static inline void __uart_write(uint16_t port, buffer_t *buf, size_t max)
{
   while(buf->sz < max)
   {
      if(__uart_can_send(port))
      {
	 __uart_send_char(port, buf->data.u8[buf->sz]);
	 buf->sz++;
      }
   }
}

size_t uart_read(uint8_t *data, size_t n)
{
   buffer_t buf;

   buf.data.u8 = data;
   buf.sz = 0;

   __uart_read(SERIAL_COM1, &buf, n);
   return buf.sz;
}

size_t uart_write(uint8_t *data, size_t n)
{
   buffer_t buf;

   buf.data.u8 = data;
   buf.sz = 0;

   __uart_write(SERIAL_COM1, &buf, n);
   return buf.sz;
}

void uart_flush()
{
   size_t s = 16;

   while(s)
      if(__uart_can_send(SERIAL_COM1))
      {
	 __uart_send_char(SERIAL_COM1, 0);
	 s--;
      }
}
