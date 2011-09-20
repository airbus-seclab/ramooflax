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
#include <cpu.h>
#include <mtrr.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void vmm_cpu_init_skillz()
{
   info->vmm.cpu.skillz.pg_1G = page_1G_supported();

   debug(CPU, "vmm 1GB pages support: %s\n"
	 ,info->vmm.cpu.skillz.pg_1G?"yes":"no");
}


static void vmm_cpu_init()
{
   vmm_cpu_init_skillz();
}

void cpu_init()
{
   vmm_cpu_init();
   vm_cpu_init();
}
