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
#include <vmx_exit_io.h>
#include <info_data.h>
#include <dev.h>
#include <debug.h>

extern info_data_t *info;

int vmx_vmexit_resolve_io()
{
   if(!dev_access())
      return 0;

   info->vm.cpu.emu_done = 1;
   vmcs_read(vm_exit_info.insn_len);
   vm_update_rip(vm_exit_info.insn_len.raw);
   return 1;
}

int __vmx_io_init(io_insn_t *io)
{
   vmcs_exit_info_io_t      *vmx_io;
   vmcs_exit_info_insn_io_t *vmx_io_s;

   vmcs_read(vm_exit_info.qualification);
   vmx_io = &vm_exit_info.qualification.io;

   io->in   = vmx_io->d;
   io->s    = vmx_io->s;
   io->sz   = vmx_io->sz+1;
   io->port = vmx_io->port;

   if(!io->s)
      return 1;

   vmcs_read(vm_exit_info.insn_info);
   vmcs_read(vm_exit_info.guest_linear);
   vmx_io_s = &vm_exit_info.insn_info.io;

   io->addr = 1<<vmx_io_s->addr;
   io->back = vm_state.rflags.df;
   io->rep  = vmx_io->rep;
   io->msk  = (1ULL<<(16*io->addr)) - 1;

   return 1;
}
