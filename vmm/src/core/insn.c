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
#include <insn.h>
#include <vmm.h>
#include <debug.h>
#include <intr.h>
#include <info_data.h>

extern info_data_t *info;

int resolve_invd()
{
   asm volatile("invd");
   info->vm.cpu.emu_done = 1;
   vm_update_rip(INVD_INSN_SZ);
   return 1;
}

int resolve_wbinvd()
{
   asm volatile ("wbinvd");
   info->vm.cpu.emu_done = 1;
   vm_update_rip(WBINVD_INSN_SZ);
   return 1;
}

int resolve_hlt()
{
   info->vm.cpu.emu_done = 1;
   return 0;
}

int resolve_icebp()
{
   info->vm.cpu.emu_done = 1;
   return 0;
}

int resolve_default()
{
   return 0;
}
