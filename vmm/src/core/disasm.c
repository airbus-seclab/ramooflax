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
#include <disasm.h>
#include <vmm.h>
#include <vm.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int __get_insn(offset_t *vaddr, int *mode)
{
   vm_get_code_addr(vaddr, 0, mode);

   if(!vm_read_mem(*vaddr, info->vm.cpu.insn_cache, sizeof(info->vm.cpu.insn_cache)))
   {
      debug(DIS, "cannot retrieve insn !\n");
      return 0;
   }

   return 1;
}

int disassemble(ud_t *disasm)
{
   offset_t vaddr;
   int      mode;

   if(!__get_insn(&vaddr, &mode))
      return 0;

   ud_init(disasm);
   ud_set_input_buffer(disasm, info->vm.cpu.insn_cache, sizeof(info->vm.cpu.insn_cache));
   ud_set_mode(disasm, mode);
   ud_set_syntax(disasm, UD_SYN_ATT);
   ud_set_vendor(disasm, UD_VENDOR_AMD);

   if(!ud_disassemble(disasm))
   {
      debug(DIS, "unable to disasm @ 0x%X !\n", vaddr);
      return 0;
   }

   debug(DIS_INSN, "@ 0x%X: \"%s\"\n", vaddr, ud_insn_asm(disasm));
   return 1;
}
