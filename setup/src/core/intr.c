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
#include <intr.h>
#include <segmem.h>
#include <info_data.h>

#include <print.h>

extern info_data_t *info;

void intr_init()
{
   idt_reg_t    idtr;
   int64_desc_t *idt;
   offset_t     isr;
   size_t       i;

   idt = info->vmm.cpu.sg->idt;
   isr = info->vmm.base + VMM_IDT_ISR_ALGN;

   for(i=0 ; i<VMM_IDT_NR_DESC ; i++, isr += VMM_IDT_ISR_ALGN)
      vmm_int64_desc(&idt[i], isr);

   idtr.ldesc = idt;
   idtr.limit = sizeof(info->vmm.cpu.sg->idt) - 1;
   set_idtr(idtr);
}
