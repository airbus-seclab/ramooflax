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
#include <svm_exit_int.h>
#include <intr.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

void __svm_vmexit_setup_interrupt_window_exiting(uint8_t enable, uint8_t irq)
{
   vmcb_ctrl_int_t             *virqctl;
   vmcb_ctrl_sys_insn_bitmap_t *intrcpt;

   virqctl = &info->vm.cpu.vmc->vm_vmcb.ctrls_area.int_ctrl;
   intrcpt = &info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap;

   if(enable ^ intrcpt->vintr)
   {
      intrcpt->vintr = enable?1:0;
      virqctl->v_irq = enable?1:0;

      if(enable)
	 virqctl->v_intr_vector = irq;
   }
}

void __svm_vmexit_inject_intn(uint8_t vector)
{
   vmcb_idt_delivery_t *ev = &info->vm.cpu.vmc->vm_vmcb.ctrls_area.event_injection;
   __svm_prepare_event_injection(ev, VMCB_IDT_DELIVERY_TYPE_SOFT, vector);
}

void __svm_vmexit_inject_interrupt(uint8_t vector)
{
   vmcb_idt_delivery_t *ev = &info->vm.cpu.vmc->vm_vmcb.ctrls_area.event_injection;
   __svm_prepare_event_injection(ev, VMCB_IDT_DELIVERY_TYPE_EXT, vector);
}

void __svm_vmexit_inject_virtual_interrupt(uint8_t vector)
{
   vmcb_ctrl_int_t *vintr;
   uint8_t         prio;

   if(vector < 16)
      prio = 15;
   else
      prio = vector/16;

   vintr = &info->vm.cpu.vmc->vm_vmcb.ctrls_area.int_ctrl;
   vintr->v_intr_vector = vector;
   vintr->v_intr_prio = prio;
   vintr->v_irq = 1;
}
