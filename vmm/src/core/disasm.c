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
#include <disasm.h>
#include <vmm.h>
#include <vm.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int __get_insn(offset_t *vaddr, int *mode)
{
   int rc;

   vm_get_code_addr(vaddr, 0, mode);

   /* VMM request to vm vmem
   **
   ** __vm_access_vmem() may fail if next page is not mapped:
   **
   ** 1 - the insn is big and located between 2 pages.
   ** This is an error because the OS should have mapped the necessary
   ** pages. However, the disasm engine should tell us (next step) it failed
   ** disassembling such an insn (partial bytes)
   **
   ** 2 - the insn is short and located at the end of a page
   ** (less than 15 bytes from page end) so that we can't read 15 bytes.
   ** This is not an error and __vm_access_vmem() may have successfully read
   ** bytes in the current page (VM_PARTIAL). This might be enough to correctly
   ** disasm the insn.
   **
   ** In any case, the disasm engine will raise the error if any.
   */
   rc = __vm_read_vmem(&__cr3, *vaddr, info->vm.cpu.insn_cache,
		       sizeof(info->vm.cpu.insn_cache));

   if(! (rc & (VM_DONE|VM_PARTIAL)))
   {
      debug(DIS, "can't read insn bytes (rc=%d)!\n", rc);
      return rc;
   }

   return VM_DONE;
}

int disassemble(ud_t *disasm)
{
   offset_t vaddr;
   int      mode;
   int      rc = __get_insn(&vaddr, &mode);

   if(rc != VM_DONE)
      return rc;

   ud_init(disasm);
   ud_set_input_buffer(disasm,
		       info->vm.cpu.insn_cache,
		       sizeof(info->vm.cpu.insn_cache));
   ud_set_mode(disasm, mode);
   ud_set_syntax(disasm, UD_SYN_ATT);
   ud_set_vendor(disasm, UD_VENDOR_AMD);

   if(!ud_disassemble(disasm))
   {
      debug(DIS, "unable to disasm @ 0x%X !\n", vaddr);
      return VM_FAIL;
   }

   debug(DIS_INSN, "@ 0x%X: \"%s\"\n", vaddr, ud_insn_asm(disasm));
   return VM_DONE;
}
