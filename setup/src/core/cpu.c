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

static void vmm_cpu_skillz_init()
{
   if(cpuid_max_ext() < CPUID_MAX_ADDR)
      panic("Not enough CPUID extensions supported");

   cpu_max_addr();

   info->vmm.cpu.skillz.pg_1G   = page_1G_supported();
   info->vmm.cpu.skillz.osxsave = osxsave_supported();

   if(info->vmm.cpu.skillz.osxsave)
   {
      cr4_reg_t cr4 = { .raw = get_cr4()|CR4_OSXSAVE };
      set_cr4(cr4);
   }

   debug(CPU,
	 "\n- vmm cpu features\n"
	 "1GB pages support   : %s\n"
	 "osxsave enabled     : %s\n"
	 "max physical addr   : 0x%X\n"
	 "max linear addr     : 0x%X\n"
	 ,info->vmm.cpu.skillz.pg_1G?"yes":"no"
	 ,info->vmm.cpu.skillz.osxsave?"yes":"no"
	 ,info->vmm.cpu.max_paddr
	 ,info->vmm.cpu.max_vaddr);
}

void cpu_init()
{
   vmm_cpu_skillz_init();
   vm_cpu_skillz_init();
}
