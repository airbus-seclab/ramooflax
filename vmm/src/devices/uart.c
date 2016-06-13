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
#include <vm.h>
#include <intr.h>
#include <dev_uart.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static inline void __dev_uart_flush(uart_t *uart)
{
   debug(DEV_UART, "tx ");
   debug_write(uart->buffer, uart->index);
   uart->index = 0;
}

static inline int __dev_uart_tx(uart_t *uart, io_insn_t *io)
{
   io_size_t sz;
   int       rc;

   do
   {
      sz.available = DEV_UART_BUFF_LEN - uart->index;

      rc = dev_io_insn(io, &uart->buffer[uart->index], &sz);
      if(rc != VM_DONE)
         return rc;

      uart->index += sz.done;

      if(uart->index >= DEV_UART_BUFF_LEN)
      {
         uart->index = DEV_UART_BUFF_LEN;
         __dev_uart_flush(uart);
      }

   } while(sz.miss);

   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_dla(uart_t *uart, io_insn_t *io)
{
   int       rc;
   io_size_t sz = { .available = 1 };

   if(io->port == SERIAL_DLA_LSB(uart->base))
      rc = dev_io_insn(io, &uart->dla.lsb, &sz);
   else
      rc = dev_io_insn(io, &uart->dla.msb, &sz);

   debug(DEV_UART, "%s dla msb 0x%x lsb 0x%x\n"
         ,io->in?"in":"out", uart->dla.msb, uart->dla.lsb);
   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_lsr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &uart->lsr.raw, &sz);
   debug(DEV_UART, "%s lsr 0x%x\n", io->in?"in":"out", uart->lsr.raw);
   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_ier(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &uart->ier.raw, &sz);

   debug(DEV_UART, "%s ier 0x%x\n", io->in?"in":"out", uart->ier.raw);

   if(!io->in && uart->ier.thre && uart->lsr.thre && __safe_interrupts_on())
   {
      uint8_t irq = info->vm.dev.pic1_icw2 + PIC_UART1_IRQ;
      uart->iir.no_int_pending = 0;
      __inject_intr(irq);
      debug(DEV_UART, "injecting irq %d for thr empty\n", irq);
   }

   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_iir(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &uart->iir.raw, &sz);

   if(io->in && !uart->iir.no_int_pending)
      uart->iir.no_int_pending = 1;

   debug(DEV_UART, "%s iir 0x%x\n", io->in?"in":"out", uart->iir.raw);
   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_fcr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &uart->fcr.raw, &sz);
   debug(DEV_UART, "%s fcr 0x%x\n", io->in?"in":"out", uart->fcr.raw);
   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_lcr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &uart->lcr.raw, &sz);
   debug(DEV_UART, "%s lcr 0x%x\n", io->in?"in":"out", uart->lcr.raw);
   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_mcr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &uart->mcr.raw, &sz);
   debug(DEV_UART, "%s mcr 0x%x\n", io->in?"in":"out", uart->mcr.raw);
   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_msr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &uart->msr.raw, &sz);
   debug(DEV_UART, "%s msr 0x%x\n", io->in?"in":"out", uart->msr.raw);
   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_scr(uart_t *uart, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &uart->scr, &sz);
   debug(DEV_UART, "%s scr 0x%x\n", io->in?"in":"out", uart->scr);
   return rc;
}

/*
** XXX
*/
static inline int __dev_uart_rx(/*uart_t *uart,*/ io_insn_t *io)
{
   uint8_t   x  = 0;
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &x, &sz);
   debug(DEV_UART, "rx 0x%x\n", x);
   return rc;
}

static inline int __dev_uart_xfr(uart_t *uart, io_insn_t *io)
{
   if(io->in)
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
      if(io->in)
         return __dev_uart_iir(uart, io);

      return __dev_uart_fcr(uart, io);
   }

   if(io->port == SERIAL_LCR(uart->base))
      return __dev_uart_lcr(uart, io);

   if(io->port == SERIAL_MCR(uart->base))
      return __dev_uart_mcr(uart, io);

   if(io->port == SERIAL_LSR(uart->base) && io->in)
      return __dev_uart_lsr(uart, io);

   if(io->port == SERIAL_MSR(uart->base) && io->in)
      return __dev_uart_msr(uart, io);

   if(io->port == SERIAL_SCR(uart->base))
      return __dev_uart_scr(uart, io);

   debug(DEV_UART, "unsupported uart operation !\n");
   return VM_FAIL;
}
