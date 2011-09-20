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
#ifndef __SVM_EXIT_INT_H__
#define __SVM_EXIT_INT_H__

#include <types.h>

/*
** preempting while guest is in realmode
** makes irq handlers being called at
** pmode exception handlers index
** because we did not remap the pic
**
** In realmode, no exception error code
** is pushed. We must inform idt handler
** that we are preempting realmode
** to get a standard stack layout
**
*/
#define preempt()				\
   ({						\
      irq_msg.rmode   = __rmode()?1:0;		\
      irq_msg.preempt = 1;			\
      asm volatile ("sti ; stgi ; clgi ; cli");	\
      irq_msg.preempt = 0;			\
   })

/*
** Functions
*/
/* void  __svm_vmexit_setup_interrupt_window_exiting(uint8_t, uint8_t); */
/* void  __svm_vmexit_inject_virtual_interrupt(uint8_t); */

#define __svm_vmexit_inject_intn(_vector)			\
   __svm_prepare_event_injection(vm_ctrls.event_injection,	\
				 VMCB_IDT_DELIVERY_TYPE_SOFT,	\
				 _vector)

#define __svm_vmexit_inject_interrupt(_vector)				\
   __svm_prepare_event_injection(vm_ctrls.event_injection,		\
				 VMCB_IDT_DELIVERY_TYPE_EXT,		\
				 _vector)

#endif
