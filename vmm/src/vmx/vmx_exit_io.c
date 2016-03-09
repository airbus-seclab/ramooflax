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
#include <vmx_exit_io.h>
#include <emulate.h>
#include <dev.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

int vmx_vmexit_resolve_io()
{
   int rc = dev_access();

   if(rc != VM_DONE)
      return rc;

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
   io->rep  = vmx_io->rep;
   io->port = vmx_io->port;

   /*
   ** XXX: to be removed, vmx does not provide bad info
   ** (maybe vmware ?)
   */
   if(io->sz != 1 && io->sz != 2 && io->sz != 4)
   {
      debug(VMX_IO, "invalid io size (%d)\n", io->sz);
      return VM_FAIL;
   }

   if(!io->s)
   {
      io->cnt = 1;
      return VM_DONE;
   }

   vmcs_read(vm_exit_info.guest_linear);

#ifdef CONFIG_VMX_FEAT_EXIT_EXT_IO
   vmcs_exit_info_insn_io_t *vmx_io_s;

   vmcs_read(vm_exit_info.insn_info);
   vmx_io_s = &vm_exit_info.insn_info.io;

   io->seg  = vmx_io_s->seg;
   io->addr = 1<<vmx_io_s->addr;
#else
   ud_t *insn = &info->vm.cpu.disasm;
   int   rc   = disassemble(insn);

   if(rc != VM_DONE)
      return rc;

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

   if(insn->pfx_seg)
      io->seg = insn->pfx_seg - UD_R_ES;
   else if(io->in)
      io->seg = IO_S_PFX_ES;
   else
      io->seg = IO_S_PFX_DS;
#endif

   if(io->seg > 5)
   {
      debug(VMX_IO, "invalid io seg pfx %d\n", io->seg);
      return VM_FAIL;
   }

   io->back = vm_state.rflags.df;
   io->msk  = (-1ULL)>>(64 - 16*io->addr);
   io->cnt  = io->rep ? (info->vm.cpu.gpr->rcx.raw & io->msk) : 1;

   return VM_DONE;
}
