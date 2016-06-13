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
#ifndef __DEVICES_IO_PORTS_H__
#define __DEVICES_IO_PORTS_H__

/*
** BOCHS
*/
#define BOCHS_ROM_START_PORT        0x400
#define BOCHS_ROM_END_PORT          0x403

#define BOCHS_VGA_START_PORT        0x500
#define BOCHS_VGA_END_PORT          0x503

/*
** DMA
*/
#define DMA1_START_PORT   0x0
#define DMA1_END_PORT     0x1f

#define DMA_PAGE_REG_START_PORT 0x80
#define DMA_PAGE_REG_END_PORT   0x8f

#define DMA2_START_PORT 0xc0
#define DMA2_END_PORT 0xdf

/*
** System Control Ports
*/
#define PS2_SYSTEM_CTRL_A_PORT   0x92

/*
** PIC
*/
#define PIC1_START_PORT 0x20
#define PIC1_END_PORT   (PIC1_START_PORT+1)

#define PIC2_START_PORT 0xa0
#define PIC2_END_PORT   (PIC2_START_PORT+1)

/*
** PIT
*/
#define PIT_START_PORT     0x40
#define PIT_END_PORT       0x5f
#define PIT_SYS_CTRL_PORT  0x61

/*
** KBD
*/
#define KBD_START_PORT 0x60
#define KBD_END_PORT 0x6f

/*
** CMOS
*/
#define CMOS_START_PORT 0x70
#define CMOS_END_PORT 0x7f

/*
** Maths Copro
*/
#define CLEAR_80287_PORT 0xf0
#define RESET_80287_PORT 0xf1

#define MATH_80287_START_PORT 0xf8
#define MATH_80287_END_PORT 0xff

/*
** SCSI
*/
#define SCSI1_START_PORT 0x340
#define SCSI1_END_PORT 0x35f

#define SCSCI2_START_PORT 0x140
#define SCSCI2_END_PORT 0x15f

/*
** ATA
*/
#define ATA2_START_PORT  0x170
#define ATA2_END_PORT    0x177
#define ATA2_CTRL_PORT   0x376

#define ATA1_START_PORT  0x1f0
#define ATA1_END_PORT    0x1f7
#define ATA1_CTRL_PORT   0x3f6

#define ATA3_START_PORT  0x1e8
#define ATA3_END_PORT    0x1ef
#define ATA3_CTRL_PORT   0x3ee

#define ATA4_START_PORT  0x168
#define ATA4_END_PORT    0x16f
#define ATA4_CTRL_PORT   0x36e


/*
** Floppy
*/
#define FLOPPY2_START_PORT  0x370
#define FLOPPY2_END_PORT    0x375

#define FLOPPY1_START_PORT  0x3f0
#define FLOPPY1_END_PORT    0x3f5

/*
** Serial
*/
#define COM4_START_PORT 0x2e8
#define COM4_END_PORT 0x2ef

#define COM2_START_PORT 0x2f8
#define COM2_END_PORT 0x2ff


//CAN BE CONFLICTING WITH IDE ATA3
#define COM3_START_PORT 0x3e8
#define COM3_END_PORT 0x3ef

#define COM1_START_PORT 0x3f8
#define COM1_END_PORT 0x3ff



/*
** Video
*/
#define EGA_ALTERNATE_START_PORT 0x2b0
#define EGA_ALTERNATE_END_PORT 0x2df

#define EGA_START_PORT 0x3c0
#define EGA_END_PORT 0x3cf

#define CGA_START_PORT 0x3d0
#define CGA_END_PORT 0x3df

/*
** Sound & Game
*/

#define GAME_START_PORT 0x200
#define GAME_END_PORT 0x20f

#define SOUND_START_PORT 0x220
#define SOUND_END_PORT 0x233

#define MPU401_START_PORT 0x330
#define MPU401_END_PORT 0x331

/*
** Printers
*/
#define LPT2_START_PORT 0x278
#define LPT2_END_PORT 0x27f

#define LPT1_START_PORT 0x378
#define LPT1_END_PORT 0x37f

/*
** Divers
*/
#define PCJR_DISK_CTL_START_PORT 0xf0
#define PCJR_DISK_CTL_END_PORT 0xf5


#endif
