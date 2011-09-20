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
#ifndef __MULTIBOOT_H__
#define __MULTIBOOT_H__

#include <grub_mbi.h>
#include <types.h>

typedef multiboot_info_t        mbi_t;
typedef multiboot_module_t      module_t;
typedef multiboot_memory_map_t  memory_map_t;

#define MBI_FLAG_MEM            MULTIBOOT_INFO_MEMORY
#define MBI_FLAG_BOOT           MULTIBOOT_INFO_BOOTDEV
#define MBI_FLAG_CMDLINE        MULTIBOOT_INFO_CMDLINE
#define MBI_FLAG_MODS           MULTIBOOT_INFO_MODS
#define MBI_FLAG_MMAP           MULTIBOOT_INFO_MEM_MAP

#define MBH_MAGIC		MULTIBOOT_HEADER_MAGIC
#define MBH_FLAGS		(MBI_FLAG_MEM|MBI_FLAG_BOOT)

#define __mbh__                 __attribute__ ((section(".mbh"),aligned(4)))

/*
** Functions
*/
typedef int (*mbi_opt_hdl_t)(char*, void*);

int  mbi_get_opt(module_t*, char*, mbi_opt_hdl_t, void*);

#endif

