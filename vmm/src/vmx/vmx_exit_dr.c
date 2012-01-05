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
#include <vmx_exit_dr.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

int vmx_vmexit_resolve_dr_access()
{
   vmcs_exit_info_dr_t *access;
   uint8_t             gpr;

   vmcs_read(vm_exit_info.qualification);
   access = &vm_exit_info.qualification.dr;
   gpr = GPR64_RAX - (access->gpr & GPR64_RAX);

   if(__resolve_dr(!access->dir, access->nr, gpr) == DR_SUCCESS)
   {
      info->vm.cpu.emu_done = 1;
      vmcs_read(vm_exit_info.insn_len);
      vm_update_rip(vm_exit_info.insn_len.raw);
      return 1;
   }

   return 0;
}
