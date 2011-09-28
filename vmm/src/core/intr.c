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
#include <vmm.h>
#include <vm.h>
#include <debug.h>
#include <emulate.h>
#include <gdb.h>
#include <info_data.h>

extern info_data_t *info;
       irq_msg_t   irq_msg;

void __regparm__(1) intr_hdlr(int64_r0_ctx_t *ctx)
{
   irq_msg.vector = ctx->nr.blow;

   if(!irq_msg.preempt && irq_msg.vector < NR_EXCP)
      vmm_excp_hdlr(ctx);
}

/*
** We only trap interrupts
** when they are enabled
** into the guest, this
** prevents us from managing
** eoi to apic/pic ...
*/
int resolve_intr()
{
   preempt();

#ifdef __CTRL_ACTIVE__
   if(gdb_singlestep_check())
   {
      __emulate_hard_interrupt(irq_msg.vector);
      return gdb_singlestep_fake();
   }
#endif

   __inject_intr(irq_msg.vector);
   return 1;
}
