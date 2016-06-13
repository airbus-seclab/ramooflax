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
#include <emulate.h>
#include <emulate_insn.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

int emulate_done(int rc, size_t sz)
{
   switch(rc)
   {
   case VM_DONE:
      vm_update_rip(sz);
   case VM_DONE_LET_RIP:
      info->vm.cpu.emu_sts = EMU_STS_DONE;
      __clear_interrupt_shadow();
      break;
   }

   return rc;
}

int emulate()
{
   size_t sz;
   int    rc = disassemble(&info->vm.cpu.disasm);

   if(rc != VM_DONE)
      return rc;

   rc = emulate_insn(&info->vm.cpu.disasm);
   sz = ud_insn_len(&info->vm.cpu.disasm);

   return emulate_done(rc, sz);
}
