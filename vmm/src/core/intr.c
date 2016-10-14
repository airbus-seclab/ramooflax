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
#include <intr.h>
#include <vmm.h>
#include <vm.h>
#include <debug.h>
#include <emulate.h>
#include <info_data.h>

extern info_data_t *info;
       irq_msg_t    irq_msg;

void __regparm__(1) intr_hdlr(int64_r0_ctx_t *ctx)
{
   irq_msg.vector = ctx->nr.blow;

   if(!irq_msg.preempt && irq_msg.vector < NR_EXCP)
      vmm_excp_hdlr(ctx);
}

/*
** On previous versions (and AMD)
** we used to keep int pending
** and sti in the vmm so that
** the interrupt is delivered inside the VMM
**
** However, now we focus only on Intel
** we ack-on-exit so that the interrupt vector
** is known without having to deal with the interrupt
** inside the VMM
**
** We still wait for interrupts being enabled inside
** the VM to inject the event.
**
** This is Intel VMX centric behavior. Under AMD
** we should "preempt()" first to acquire interrupt
** under the vmm.
*/
int resolve_intr()
{
   /* XXX: singlestep checks */

   if(irq_msg.vector == 0xc0)
      panic("woot IO MMU fault");

   /*
   ** - We can't inject for now, so make it pending
   ** - No further interrupts can be raised
   **   as we do not manage interrupt controller
   **   (pic/apic) and do not send EOI
   */
   if(!__safe_interrupts_on())
      __setup_iwe(1, irq_msg.vector);
   else
      __inject_intr(irq_msg.vector);

   return VM_DONE;
}
