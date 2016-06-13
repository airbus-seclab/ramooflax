/*
** Copyright (C) 2016 Airbus Group, stephane duverger <stephane.duverger@airbus.com>
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
#include <emulate.h>
#include <dev.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

int svm_vmexit_resolve_io()
{
   int rc = dev_access();

   if(rc != VM_DONE)
      return rc;

   vm_state.rip.raw = vm_ctrls.exit_info_2.raw;
   return emulate_done(VM_DONE, 0);
}

int __svm_io_init(io_insn_t *io)
{
   svm_io_t *svm = &vm_ctrls.exit_info_1.io;

   io->in   = svm->d;
   io->s    = svm->s;
   io->sz   = (svm->low>>4) & 7;
   io->port = svm->port;

   if(!io->s)
   {
      io->cnt = 1;
      return VM_DONE;
   }

   if(svm->seg > 5)
   {
      debug(SVM_IO, "invalid io seg pfx %d\n", svm->seg);
      return VM_FAIL;
   }

   io->seg  = svm->seg;
   io->addr = (svm->low>>7) & 7;
   io->back = vm_state.rflags.df;
   io->msk  = (-1ULL)>>(64 - 16*io->addr);
   io->rep  = svm->rep;
   io->cnt  = io->rep ? (info->vm.cpu.gpr->rcx.raw & io->msk) : 1;

   return VM_FAIL;
}
