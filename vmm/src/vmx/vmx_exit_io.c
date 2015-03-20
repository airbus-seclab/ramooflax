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
#include <emulate.h>
#include <dev.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

int vmx_vmexit_resolve_io()
{
   if(!dev_access())
      return VM_FAIL;

   vmcs_read(vm_exit_info.insn_len);
   return emulate_done(VM_DONE, vm_exit_info.insn_len.raw);
}

int __vmx_io_init(io_insn_t *io)
{
   vmcs_exit_info_io_t *vmx_io;

   vmcs_read(vm_exit_info.qualification);
   vmx_io = &vm_exit_info.qualification.io;

   io->in   = vmx_io->d;
   io->s    = vmx_io->s;
   io->sz   = vmx_io->sz+1;
   io->port = vmx_io->port;

   if(!io->s)
      return 1;

   vmcs_read(vm_exit_info.guest_linear);

#ifdef CONFIG_VMX_FEAT_EXIT_EXT_IO
   vmcs_exit_info_insn_io_t *vmx_io_s;

   vmcs_read(vm_exit_info.insn_info);
   vmx_io_s = &vm_exit_info.insn_info.io;
   io->addr = 1<<vmx_io_s->addr;
#else
   ud_t *insn = &info->vm.cpu.disasm;

   if(!disassemble(insn))
      return 0;

   if(insn->dis_mode == 64)
   {
      if(insn->pfx_adr)
	 io->addr = 2;
      else
	 io->addr = 4;
   }
   else if(insn->dis_mode == 32)
   {
      if(insn->pfx_adr)
	 io->addr = 1;
      else
	 io->addr = 2;
   }
   else
   {
      if(insn->pfx_adr)
	 io->addr = 2;
      else
	 io->addr = 1;
   }
#endif

   io->back = vm_state.rflags.df;
   io->rep  = vmx_io->rep;
   io->msk  = (1ULL<<(16*io->addr)) - 1;

   return 1;
}
