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
#include <elf.h>
#include <print.h>
#include <string.h>

/*
** This code works under 32 or 64 bits
** However, 32 bits code truncates offsets
** that can not fit into 32 bits addresses
** by making use of "offset_t" which is
** "unsigned long" (4 bytes under 32 bits).
*/
static Elf64_Shdr* __elf_section(Elf64_Ehdr *e_hdr, Elf64_Word type)
{
   Elf64_Half n;
   Elf64_Shdr *s_hdr;
   loc_t      sloc;

   sloc.linear = (offset_t)e_hdr + (offset_t)e_hdr->e_shoff;
   for(n=0 ; n<e_hdr->e_shnum ; n++)
   {
      s_hdr = (Elf64_Shdr*)sloc.addr;

      if(s_hdr->sh_type == type)
         return s_hdr;

      sloc.linear += (offset_t)e_hdr->e_shentsize;
   }

   return (Elf64_Shdr*)0;
}

static void __elf_module_load(module_t *mod, offset_t base, offset_t not_before)
{
   Elf64_Ehdr *e_hdr;
   Elf64_Phdr *p_hdr;
   Elf64_Shdr *b_hdr;
   sint64_t   pad;
   loc_t      file, mem;

   e_hdr = (Elf64_Ehdr*)((offset_t)mod->mod_start);
   p_hdr = (Elf64_Phdr*)((offset_t)e_hdr + (offset_t)e_hdr->e_phoff);

   if(e_hdr->e_machine != EM_X86_64 || ! e_hdr->e_phnum || p_hdr->p_type != PT_LOAD)
      panic("invalid elf module");

   mem.linear = base + (offset_t)p_hdr->p_vaddr;
   if(mem.linear < not_before)
      panic("elf overlaps mbi modules");

   file.linear = (offset_t)mod->mod_start + (offset_t)p_hdr->p_offset;
   memcpy(mem.addr, file.addr, (offset_t)p_hdr->p_filesz);

   pad = p_hdr->p_memsz - p_hdr->p_filesz;
   if(pad > 0)
   {
      mem.linear += (offset_t)p_hdr->p_filesz;
      memset(mem.addr, 0, (size_t)pad);
   }

   b_hdr = __elf_section(e_hdr, SHT_NOBITS);
   if(b_hdr)
   {
      mem.linear = base + (offset_t)b_hdr->sh_addr;
      memset(mem.addr, 0, (size_t)b_hdr->sh_size);
   }
}

void elf_module_load(module_t *mod, offset_t not_before)
{
  __elf_module_load(mod, 0, not_before);
}

void elf_module_load_pic_static(module_t *mod, offset_t base)
{
  __elf_module_load(mod, base, 0);
}

void elf_module_load_relocatable(module_t *mod, offset_t base)
{
   Elf64_Ehdr  *e_hdr;
   Elf64_Shdr  *r_hdr;
   Elf64_Rela  *rel;
   Elf64_Xword n;
   loc_t       rloc, fix;

   __elf_module_load(mod, base, 0);

   e_hdr = (Elf64_Ehdr*)((offset_t)mod->mod_start);
   r_hdr = __elf_section(e_hdr, SHT_RELA);
   if(! r_hdr)
      panic("no relocation section");

   rloc.linear = (offset_t)mod->mod_start + (offset_t)r_hdr->sh_offset;
   for(n=0 ; n<(r_hdr->sh_size/r_hdr->sh_entsize) ; n++)
   {
      rel = (Elf64_Rela*)rloc.addr;

      if(ELF64_R_TYPE(rel->r_info) != R_X86_64_RELATIVE)
         panic("bad relocation entry");

      if(rel->r_addend < 0)
         panic("humm negative addend");

      fix.linear   = base + (offset_t)rel->r_offset;
      *fix.u64     = base + (offset_t)rel->r_addend;

      rloc.linear += (offset_t)r_hdr->sh_entsize;
   }
}

Elf64_Addr elf_module_entry(module_t *mod)
{
   Elf64_Ehdr *e_hdr = (Elf64_Ehdr*)((offset_t)mod->mod_start);
   return e_hdr->e_entry;
}

Elf64_Phdr* elf_module_phdr(module_t *mod)
{
   Elf64_Ehdr *e_hdr;
   Elf64_Phdr *p_hdr;

   e_hdr = (Elf64_Ehdr*)((offset_t)mod->mod_start);
   p_hdr = (Elf64_Phdr*)((offset_t)e_hdr + (offset_t)e_hdr->e_phoff);

   if(e_hdr->e_phnum && p_hdr->p_type == PT_LOAD)
      return p_hdr;

   panic("invalid elf phdr");
   return (Elf64_Phdr*)0;
}

Elf64_Xword elf_module_load_algn(module_t *mod)
{
   Elf64_Phdr *p_hdr = elf_module_phdr(mod);
   return p_hdr->p_align;
}

Elf64_Xword elf_module_load_size(module_t *mod)
{
   Elf64_Phdr *p_hdr = elf_module_phdr(mod);
   return p_hdr->p_memsz;
}
