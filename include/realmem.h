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
#ifndef __REALMEM_H__
#define __REALMEM_H__

#include <types.h>

/*
** Real mode memory map
*/

/* 1KB IVT */
#define IVT_START             0x000000
#define IVT_END               0x000400
/* 512B BIOS data area */
#define BIOS_DATA_START       0x000400
#define BIOS_DATA_END         0x000600
/* conventional memory */
#define LOW_MEM_START         0x000600
#define LOW_MEM_END           0x09fc00
/* BIOS extended data area */
#define BIOS_EXT_DATA_START   0x09fc00
#define BIOS_EXT_DATA_END     0x0a0000
/* 128KB of VGA memory */
#define VGA_START             0x0a0000
#define VGA_END               0x0c0000
/* 192KB VGA BIOS rom */
#define VGA_BIOS_START        0x0c0000
#define VGA_BIOS_END          0x0f0000
/* 64KB BIOS rom */
#define BIOS_START            0x0f0000
#define BIOS_START_OFFSET     0x00fff0
#define BIOS_END              0x100000
/* Extended memory */
#define EXT_MEM_START         (1<<20)
/* DMA limit */
#define DMA_END               (16<<20)
/* BIOS bootloader @ */
#define BIOS_BOOTLOADER       0x07c00

#define is_bios_mem(__AdDr__)					\
   (mem_range((__AdDr__),IVT_START,BIOS_DATA_END) ||		\
    mem_range((__AdDr__),BIOS_EXT_DATA_START,EXT_MEM_START))

/*
** rmode max addr is 0xffff<<4 + 0xffff == 0x10ffef == RM_WRAP_LIMIT
**
** - if a20 is off, we emulate wrap-around by mirroring
**   pages from [ 1MB ; 1MB+64KB [ to [ 0 ; 64KB [
**
** - if a20 is on, we do not wrap-around
**
*/
#define RM_LIMIT             EXT_MEM_START
#define RM_WRAP_LIMIT        (RM_LIMIT+(64<<10))
#define RM_STACK_BOTTOM      ((LOW_MEM_END & 0xffff0000) - 2)
#define RM_BASE_SS           ((RM_STACK_BOTTOM & 0xffff0000)>>4)
#define RM_BASE_SP           (RM_STACK_BOTTOM & 0xffff)
#define RM_BASE_IP           (LOW_MEM_START & 0xffff)

#endif
