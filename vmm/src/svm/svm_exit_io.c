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
#include <svm_exit_io.h>
#include <info_data.h>
#include <dev.h>
#include <debug.h>

extern info_data_t *info;

int svm_vmexit_resolve_io()
{
   if(!dev_access())
      return 0;

   info->vm.cpu.emu_done = 1;
   vm_state.rip.raw = vm_ctrls.exit_info_2.raw;
   return 1;
}

int __svm_io_init(io_insn_t *io)
{
   svm_io_t *svm = (svm_io_t*)&vm_ctrls.exit_info_1;

   io->in   = svm->io.d;
   io->s    = svm->io.s;
   io->sz   = (svm->low>>4) & 7;
   io->port = svm->io.port;

   if(!io->s)
      return 1;

   io->addr = (svm->low>>7) & 7;
   io->back = vm_state.rflags.df;
   io->rep  = svm->io.rep;
   io->msk  = (1ULL<<(16*io->addr)) - 1;

   if(svm->io.seg > 5)
   {
      debug(SVM_IO, "invalid io seg pfx %d\n", svm->io.seg);
      return 0;
   }

   return 1;
}
