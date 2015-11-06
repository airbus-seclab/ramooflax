/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#ifndef __DEV_ATA_H__
#define __DEV_ATA_H__

#include <types.h>
#include <io.h>

/*
** Default IRQ Lines
*/
#define ATA1_IRQ_LINE   14
#define ATA2_IRQ_LINE   15
#define ATA3_IRQ_LINE   11
#define ATA4_IRQ_LINE   10


/*
** Command Block Registers
*/
#define ATA_DATA_REG(BASE)         (BASE)      /* R/W */
#define ATA_ERR_REG(BASE)         ((BASE)+1)   /*  R  */
#define ATA_FEAT_REG(BASE)        ((BASE)+1)   /*  W  */
#define ATA_SECT_CNT_REG(BASE)    ((BASE)+2)   /* R/W */
#define ATA_LBA_LOW_REG(BASE)     ((BASE)+3)   /* R/W */
#define ATA_LBA_MID_REG(BASE)     ((BASE)+4)   /* R/W */
#define ATA_LBA_HIGH_REG(BASE)    ((BASE)+5)   /* R/W */
#define ATA_DEVICE_REG(BASE)      ((BASE)+6)   /* R/W */

/*
** Command/Status Register
**
** Read : Status Register
** Write: Command Register
*/
#define ATA_STATUS_REG(BASE)       ((BASE)+7)   /* R */
#define ATA_CMD_REG(BASE)          ((BASE)+7)   /* W */

/*
** ATA COMMANDS
*/
#define ATA_READ_SECTOR_CMD             0x20
#define ATA_WRITE_SECTOR_CMD            0x30
#define ATA_IDENT_DEV_CMD               0xec
#define ATA_IDENT_PKT_DEV_CMD           0xa1
#define ATA_INIT_DEV_PARAMS_CMD         0x91
#define ATA_RECALIBRATE_CMD             0x10
#define ATA_DEVICE_RESET_CMD            0x08
#define ATA_READ_DMA_CMD                0xc8
#define ATA_WRITE_DMA_CMD               0xca
#define ATA_SET_FEAT_CMD                0xef
#define ATA_SET_MULTI_MODE_CMD          0xc6

/*
** Status Register
*/
typedef union ata_status_register
{
   struct
   {
      uint8_t   err:1;   /* Error in previous command */
      uint8_t   obs:2;   /* obsolete */
      uint8_t   drq:1;   /* Data Request (ready to transfer between host/device) */
      uint8_t   gen:1;   /* Depend on command */
      uint8_t   df:1;    /* Device Fault */
      uint8_t   drdy:1;  /* Device Ready */
      uint8_t   bsy:1;   /* Device Busy */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) ata_status_reg_t;


/*
** Device Register (Drive Head Register)
*/
typedef union ata_device_register
{
   struct
   {
      uint8_t  gen:4;   /* depend on command */
      uint8_t  dev:1;   /* device select */
      uint8_t  obs:1;   /* obsolete */
      uint8_t  gen2:1;  /* depend on command (ie. LBA for read/write sector) */
      uint8_t  obs2:1;  /* obsolete */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) ata_dev_reg_t;

/*
** Error Register
*/
typedef union ata_error_register
{
   struct
   {
      uint8_t   gen:2;   /* depend on command */
      uint8_t   abrt:1;  /* (1) cmd aborted */
      uint8_t   gen2:5;  /* depend on command */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) ata_err_reg_t;



/*
** Control Block Registers
*/

/*
** Alternative Status/Device Control Register
**
** Read : Alternate Status Register
** Write: Device Control Register
*/
#define ATA_ALT_STATUS_REG(BASE)  ((BASE)+0x206) /* R */
#define ATA_DEV_CTRL_REG(BASE)    ((BASE)+0x206) /* W */

/*
** Alternate Status
**
** if BSY = 1, other bits should not be used
** Content invalid if device in sleep mode
**
** cf. status register for fields
*/

/*
** Device Control
*/
typedef union ata_device_control_register
{
   struct
   {
      uint8_t  zero:1;      /* fixed to 0 */
      uint8_t  nien:1;      /* (0) enable irq (1) disable irq */
      uint8_t  srst:1;      /* host software reset bit */
      uint8_t  reserved:4;  /* reserved */
      uint8_t  hob:1;       /* high order byte */

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) ata_dev_ctrl_reg_t;


/*
** ATA Device
*/
typedef struct ata_device
{
   uint8_t  cnt;
   uint8_t  lba[4];

} __attribute__((packed)) ata_dev_t;


/*
** ATA Controller
*/
typedef struct ata_controller
{
   uint16_t          base;
   ata_dev_reg_t     dev;
   ata_dev_t         devices[2];

} __attribute__((packed)) ata_t;

/*
** Functions
*/
#ifdef __INIT__
void dev_ata_init(ata_t*, uint16_t);
#else
int  dev_ata(ata_t*, io_insn_t*);
#endif


#endif
