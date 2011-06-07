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
#ifndef __UART_H__
#define __UART_H__

#include <types.h>

/*
** Base IO ports
*/
#define SERIAL_COM1             0x3f8
#define SERIAL_COM2             0x2f8
#define SERIAL_COM3             0x3e8
#define SERIAL_COM4             0x2e8

/* LCR.dla = 0 */
#define SERIAL_TXRX(BASE)       (BASE)
#define SERIAL_TX(BASE)         (BASE)     /* WO */
#define SERIAL_RX(BASE)         (BASE)     /* RO */
#define SERIAL_IER(BASE)       ((BASE)+1)  /* RW */

#define SERIAL_IIR(BASE)       ((BASE)+2)  /* RO */
#define SERIAL_FCR(BASE)       ((BASE)+2)  /* WO */

#define SERIAL_LCR(BASE)       ((BASE)+3)  /* RW */
#define SERIAL_MCR(BASE)       ((BASE)+4)  /* RW */
#define SERIAL_LSR(BASE)       ((BASE)+5)  /* RO */
#define SERIAL_MSR(BASE)       ((BASE)+6)  /* RO */
#define SERIAL_SCR(BASE)       ((BASE)+7)  /* RW */

/* LCR.dla = 1 */
#define SERIAL_DLA_LSB(BASE)    (BASE)     /* RW */
#define SERIAL_DLA_MSB(BASE)   ((BASE)+1)  /* RW */

/* LCR = 0xbf */
#define SERIAL_EFR(BASE)       ((BASE)+2)  /* RW */
#define SERIAL_XON1(BASE)      ((BASE)+4)  /* RW */
#define SERIAL_XON2(BASE)      ((BASE)+5)  /* RW */
#define SERIAL_XOFF1(BASE)     ((BASE)+6)  /* RW */
#define SERIAL_XOFF2(BASE)     ((BASE)+7)  /* RW */

/*
** Enhanced Feature Register
*/
typedef union serial_efr_register
{
   struct
   {
      uint8_t   flow_ctl:4; /* (00xx) no tx flow
			   ** (10xx) tx xon/xoff 1
			   ** (01xx) tx xon/xoff 2
			   ** (11xx) tx xon/xoff 1,2
			   ** (xx00) no rx flow
			   ** (xx10) rx cmp xon/xoff 1
			   ** (xx01) rx cmp xon/xoff 2
			   ** (1011) ...
			   ** (0111) ...
			   ** (1111) ...
			   */
      uint8_t   ctl:1;      /* (1) to modify IER[7-4], ISR[5-4], FCR[5-4] */
      uint8_t   spc:1;      /* special character detect */
      uint8_t   a_rts:1;    /* (1) automatic RTS */
      uint8_t   a_cts:1;    /* (1) automatic CTS */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) serial_efr_reg_t;


/*
** Interrupt Enable Register
*/
#define SERIAL_IER_RECV              (1<<0)
#define SERIAL_IER_THRE              (1<<1)
#define SERIAL_IER_RLSR              (1<<2)
#define SERIAL_IER_MSR               (1<<3)
#define SERIAL_IER_SLEEP             (1<<4)
#define SERIAL_IER_LOW               (1<<5)
#define SERIAL_IER_RESERVED1         (1<<6)
#define SERIAL_IER_RESERVED2         (1<<7)

typedef union serial_ier_register
{
   struct
   {
      uint8_t   recv:1;     /* (1) enable received data available interrupt */
      uint8_t   thre:1;     /* (1) enable THR empty interrupt */
      uint8_t   lsr:1;      /* (1) enable receiver line status interrupt */
      uint8_t   msr:1;      /* (1) enable modem status interrupt */
      uint8_t   sleep:1;
      uint8_t   xoff:1;     /* (1) enable xoff interrupt */
      uint8_t   rts:1;      /* (1) enable RTS interrupts */
      uint8_t   cts:1;      /* (1) enable CTS interrupts */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) serial_ier_reg_t;


/*
** Interrupt Info Register
*/
#define SERIAL_IIR_RECV_MASK                  2

#define SERIAL_IIR_INFO_MODEM_STATUS_CHANGE   0
#define SERIAL_IIR_INFO_THR_EMPTY             1
#define SERIAL_IIR_INFO_RECV_DATA_AVAILABLE   2
#define SERIAL_IIR_INFO_LINE_STATUS_CHANGE    3
#define SERIAL_IIR_INFO_CHAR_TIMEOUT          6

#define SERIAL_IIR_INFO_FIFO_NO               0
#define SERIAL_IIR_INFO_FIFO_USABLE           2
#define SERIAL_IIR_INFO_FIFO_ENABLED          3

typedef union serial_iir_register
{
   struct
   {
      uint8_t  no_int_pending:1;   /* (0) int pending, (1) no */
      uint8_t  info:3;             /* see info type values */
      uint8_t  reserved:2;
      uint8_t  fifo:2;             /* see fifo info values above */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) serial_iir_reg_t;

/*
** Fifo Control Register
*/

/* Some FIFO size values 

** sc16c650a values:

#define SERIAL_FCR_TX_FIFO_8      1
#define SERIAL_FCR_TX_FIFO_16     0
#define SERIAL_FCR_TX_FIFO_24     2
#define SERIAL_FCR_TX_FIFO_30     3

#define SERIAL_FCR_RX_FIFO_8      0
#define SERIAL_FCR_RX_FIFO_16     1
#define SERIAL_FCR_RX_FIFO_24     2
#define SERIAL_FCR_RX_FIFO_28     3

** 16550 values:

#define SERIAL_FCR_RX_FIFO_1      0
#define SERIAL_FCR_RX_FIFO_4      1
#define SERIAL_FCR_RX_FIFO_8      2
#define SERIAL_FCR_RX_FIFO_14     3

*/

#define SERIAL_FCR_RX_FIFO_1      0
#define SERIAL_FCR_RX_FIFO_4      1
#define SERIAL_FCR_RX_FIFO_8      2
#define SERIAL_FCR_RX_FIFO_14     3

typedef union serial_fcr_register
{
   struct
   {
      uint8_t   enable:1;     /* enable fifo */
      uint8_t   rx:1;         /* clear receive fifo */
      uint8_t   tx:1;         /* clear transmit fifo */
      uint8_t   dma:1;        /* rx/tx ready pins */
      uint8_t   tx_trigger:2;
      uint8_t   rx_trigger:2;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) serial_fcr_reg_t;


/*
** Line Control Register
*/
typedef union serial_lcr_register
{
   struct
   {
      uint8_t  word_len:2;   /* (00) 5 bits, (01) 6 bits, (10) 7 bits, (11) 8 bits */
      uint8_t  stop:1;       /* (0) 1 stop bit, (1) 1.5 or 2 stop bits */
      uint8_t  parity:3;     /* (xx0) no parity
			     ** (001) odd
			     ** (011) even
			     ** (101) mark
			     ** (111) space */
      uint8_t  brk:1;        /* (1) break enable */
      uint8_t  dla:1;        /* (1) Divisor Latch Access Bit */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) serial_lcr_reg_t;


/*
** Modem Control Register
*/
typedef union serial_mcr_register
{
   struct
   {
      uint8_t   data:1;      /* data terminal ready */
      uint8_t   req:1;       /* request to send */
      uint8_t   aux1:1;      /* auxiliary output 1 */
      uint8_t   aux2:1;      /* auxiliary output 2 */
      uint8_t   loop:1;      /* loopback mode */
      uint8_t   auto_flow:1; /* auto flow control */
      uint8_t   reserved:2;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) serial_mcr_reg_t;


/*
** Line Status Register
*/
#define SERIAL_LSR_DATA_READY_BIT    0
#define SERIAL_LSR_OVERRUN_ERR_BIT   1
#define SERIAL_LSR_PARITY_ERR_BIT    2
#define SERIAL_LSR_FRAME_ERR_BIT     3
#define SERIAL_LSR_BREAK_INT_BIT     4
#define SERIAL_LSR_THRE_BIT          5
#define SERIAL_LSR_TSRE_BIT          6
#define SERIAL_LSR_ZERO_BIT          7

#define SERIAL_LSR_DATA_READY        (1<<SERIAL_LSR_DATA_READY_BIT)
#define SERIAL_LSR_OVERRUN_ERR       (1<<SERIAL_LSR_OVERRUN_ERR_BIT)
#define SERIAL_LSR_PARITY_ERR        (1<<SERIAL_LSR_PARITY_ERR_BIT)
#define SERIAL_LSR_FRAME_ERR         (1<<SERIAL_LSR_FRAME_ERR_BIT)
#define SERIAL_LSR_BREAK_INT         (1<<SERIAL_LSR_BREAK_INT_BIT)
#define SERIAL_LSR_THRE              (1<<SERIAL_LSR_THRE_BIT)
#define SERIAL_LSR_TSRE              (1<<SERIAL_LSR_TSRE_BIT)
#define SERIAL_LSR_ZERO              (1<<SERIAL_LSR_ZERO_BIT)

typedef union serial_lsr_register
{
   struct
   {
      uint8_t   data:1;      /* data ready */
      uint8_t   overrun:1;   /* over run */
      uint8_t   parity:1;    /* parrity error */
      uint8_t   fram:1;      /* framing error */
      uint8_t   brk:1;       /* break interrupt */
      uint8_t   thre:1;      /* empty THR */
      uint8_t   tsre:1;      /* empty TSR */
      uint8_t   fifo:1;      /* error in received fifo */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) serial_lsr_reg_t;

/*
** Modem Status Register
*/
typedef union serial_msr_register
{
   struct
   {
      uint8_t   dcts:1;    /* delta clear to send */
      uint8_t   ddsr:1;    /* delta data set ready */
      uint8_t   teri:1;    /* trailing edge ring indicator */
      uint8_t   ddcd:1;    /* delta data carrier detect */
      uint8_t   cts:1;     /* clear to send */
      uint8_t   dsr:1;     /* data set ready */
      uint8_t   ri:1;      /* ring indicator */
      uint8_t   cd:1;      /* carrier detect */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) serial_msr_reg_t;


/*
** Divisor Latch Access
*/
typedef union serial_dla
{
   struct
   {
      uint8_t     lsb;
      uint8_t     msb;

   } __attribute__((packed));

   uint16_t raw;

} __attribute__((packed)) serial_dla_t;


/*
** Functions
*/
#define __uart_send_char(BASE,c)     out((c), SERIAL_TX((BASE)))
#define __uart_recv_char(BASE)       in(SERIAL_TX((BASE)))
#define __uart_can_send(BASE)	     (in(SERIAL_LSR((BASE))) & SERIAL_LSR_THRE)
#define __uart_can_recv(BASE)	     (in(SERIAL_LSR((BASE))) & SERIAL_LSR_DATA_READY)

#define uart_enable_dla_registers(BASE)     out( 0x80, SERIAL_LCR((BASE)) )
#define uart_set_lsb_dla_rate(BASE,x)       out( (x),  SERIAL_DLA_LSB((BASE)) )
#define uart_set_msb_dla_rate(BASE,x)       out( (x),  SERIAL_DLA_MSB((BASE)) )
#define uart_enable_efr_registers(BASE)     out( 0xbf, SERIAL_LCR((BASE)) )

#ifdef __INIT__
void  uart_init();
#endif

size_t  uart_read(uint8_t*, size_t);
size_t  uart_write(uint8_t*, size_t);

#endif
