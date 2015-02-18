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
#include <show.h>
#include <print.h>
#include <info_data.h>

extern info_data_t *info;

void show_vmm_mem_map()
{
   printf("\n- vmm physical memory map\n"
          "area start    : 0x%X\n"
          "area end      : 0x%X\n"
          "area size     : %D B (%D KB)\n"
          "vmm stack     : 0x%X\n"
          "vmm pool      : 0x%X (%D KB)\n"
          "vmm elf       : 0x%X - 0x%X (%D B)\n"
          "gdt           : 0x%X\n"
          "idt           : 0x%X\n"
          "pml4          : 0x%X\n"
          "vm vmc        : 0x%X\n"
          ,info->area.start
          ,info->area.end
          ,info->area.size, (info->area.size)>>10
          ,info->vmm.stack_bottom
          ,info->vmm.pool.addr, (info->vmm.pool.sz)>>10
          ,info->vmm.base, info->vmm.base+info->vmm.size, info->vmm.size
          ,info->vmm.cpu.sg->gdt
          ,info->vmm.cpu.sg->idt
          ,info->vmm.cpu.pg.pml4
          ,info->vm.cpu.vmc
      );
}
