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
#ifndef __PIC_H__
#define __PIC_H__

#include <types.h>

/*
** Intel 8259a
*/

#define PIC1                  0x20
#define PIC2                  0xa0

/*
** Convert PIC irq line number
** to real mode IVT index
** based on default BIOS settings
*/
#define DFLT_PIC1_ICW2        0x8
#define DFLT_PIC2_ICW2        0x70
#define irq_to_ivt(x)         (((x)<8)?((x)+DFLT_PIC1_ICW2):((x-8)+DFLT_PIC2_ICW2))

#define PIC_IRQ_NR            16

/*
** Devices linked to PICs
*/
#define PIC_TIMER_IRQ         0
#define PIC_KBD_IRQ           1
#define PIC_SLAVE_IRQ         2
#define PIC_UART2_IRQ         3
#define PIC_UART1_IRQ         4
#define PIC_SND_IRQ           5
#define PIC_FD_IRQ            6
#define PIC_PP_IRQ            7

#define PIC_RTC_IRQ           0
#define PIC_PS2_IRQ           4
#define PIC_MATH_IRQ          5
#define PIC_HDD_IRQ           6

/* A0 line = 0 */
#define PIC_ICW1(BASE)       (BASE)     /*  W  */
#define PIC_OCW2(BASE)       (BASE)     /*  W  */
#define PIC_OCW3(BASE)       (BASE)     /*  W  */
#define PIC_IRR(BASE)        (BASE)     /*  R  */
#define PIC_ISR(BASE)        (BASE)     /*  R  */

#define is_pic_icw1(_dAtA_)  ((_dAtA_)&0x10)
#define is_pic_ocw2(_dAtA_)  (((_dAtA_)&0x18) == 0 )
#define is_pic_ocw3(_dAtA_)  (((_dAtA_)&0x98) == 8 )

#define PIC_EOI               0x20      /* OCW2 non specific EOI */

/* A0 line = 1 */
#define PIC_IMR(BASE)       ((BASE)+1)  /* R/W */
#define PIC_ICW2(BASE)      ((BASE)+1)  /*  W  */
#define PIC_ICW3(BASE)      ((BASE)+1)  /*  W  */
#define PIC_ICW4(BASE)      ((BASE)+1)  /*  W  */

#define pic_eoi(BASE)       out(PIC_EOI,BASE)
#define pic_imr(BASE)       in(PIC_IMR(BASE))

/*
** PIC Initialization command word 1
*/
typedef union pic_init_command_word_1
{
   struct
   {
      uint8_t   icw4:1;    /* (1) need icw4 */
      uint8_t   single:1;  /* (1) single PIC (0) cascaded */
      uint8_t   unused:1;  /* unused on x86 */
      uint8_t   ltim:1;    /* (1) level triggered interrupt mode (0) edge triggered interrupt mode */
      uint8_t   one:1;     /* MUST BE SET TO 1 */
      uint8_t   vect:3;    /* Interrupt Vector Addresses for MCS mode */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) pic_icw1_t;

/*
** PIC Initialization command word 2
**
** Base IRQ number for PIC1 and PIC2
** Each IRQ delivered by each PIC
** will be added to this base to form
** the final IRQ number
**
** For PIC 1:
** In real mode: IRQ0 is at IVT[8]
** In prot mode: IRQ0 is at IDT[32]
**
** The 3 less significant bits are
** always zero when writing ICW2.
** They correspond to irq number (0-7)
** related to the PIC
**
** This allow processor to associate
** IRQ with processor interrupt handler
**
*/
typedef union pic_init_command_word_2
{
   struct
   {
      uint8_t  zero:3;
      uint8_t  irq_base:5;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) pic_icw2_t;

/*
** PIC Initialization command word 3
**
** Slave connection
*/
typedef union pic_init_command_word_3
{
   struct
   {
      uint8_t   irq0:1;     /* irq0 is connected to a slave */
      uint8_t   irq1:1;
      uint8_t   irq2:1;
      uint8_t   irq3:1;
      uint8_t   irq4:1;
      uint8_t   irq5:1;
      uint8_t   irq6:1;
      uint8_t   irq7:1;
   } master;

   struct
   {
      uint8_t   id:3; /* master irq number connected to slave */
      uint8_t   zero:5;
   } slave;

   uint8_t      raw;

} __attribute__((packed)) pic_icw3_t;

/*
** PIC Initialization command word 4
*/
typedef union pic_init_command_word_4
{
   struct
   {
      uint8_t   x86:1;    /* (1) 8086  (0) MCS */
      uint8_t   aeoi:1;   /* (1) Auto EOI  (0) Normal EOI */
      uint8_t   ms:1;     /* (1) master buffered (0) slave buffered */
      uint8_t   buff:1;   /* (1) buffered mode (0) non buffered mode */
      uint8_t   sfnm:1;   /* (1) Special Fully Nested Mode (0) Fully Nested Mode */
      uint8_t   zero:3;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) pic_icw4_t;

/*
** PIC Operation control word 1
**
** Control Interrupt Mask Register
**
** (0) irq active
** (1) irq inactive
*/
typedef union pic_operation_control_word_1
{
   struct
   {
      uint8_t  irq0:1;
      uint8_t  irq1:1;
      uint8_t  irq2:1;
      uint8_t  irq3:1;
      uint8_t  irq4:1;
      uint8_t  irq5:1;
      uint8_t  irq6:1;
      uint8_t  irq7:1;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) pic_ocw1_t;

/*
** PIC Operation control word 2
*/
#define PIC_OCW2_DISABLE_ROTATE_AEOI       0
#define PIC_OCW2_NON_SPECIFIC_EOI          1
#define PIC_OCW2_NO_OPERATION              2
#define PIC_OCW2_SPECIFIC_EOI              3
#define PIC_OCW2_ENABLE_ROTATE_AEOI        4
#define PIC_OCW2_ROTATE_NON_SPECIFIC_EOI   5
#define PIC_OCW2_SET_PRIORITY              6
#define PIC_OCW2_ROTATE_SPECIFIC_EOI       7

typedef union pic_operation_control_word_2
{
   struct
   {
      uint8_t   lvl:3;   /* interrupt level when SL is set */
      uint8_t   zero:2;
      uint8_t   cmd:3;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) pic_ocw2_t;

/*
** PIC Operation control word 3
**
** Read Status Registers
*/
#define PIC_OCW3_SPECIAL_MASK_MODE_ENABLED   3
#define PIC_OCW3_SPECIAL_MASK_MODE_DISABLED  2

typedef union pic_operation_control_word_3
{
   struct
   {
      uint8_t   reg:2;   /* (00) reserved
                         ** (01) reserved
                         ** (10) Next Read on Base Register returns Interrupt Request Register
                         ** (11) Next Read on Base Register returns In-Service Register
                         */
      uint8_t   poll:1;  /* (1) Poll command, (0) No poll command */
      uint8_t   one:1;   /* MUST BE SET TO 1 */
      uint8_t   zero:1;
      uint8_t   smm:2;   /* (00) reserved
                         ** (01) reserved
                         ** (10) Reset Special Mask Mode
                         ** (11) Set Special Mask Mode
                         */
      uint8_t   zero2:1;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) pic_ocw3_t;

/*
** Functions
*/
#ifdef __INIT__
void pic_init();
#endif

#endif

