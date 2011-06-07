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
#include <excp.h>
#include <intr.h>
#include <gdb.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

char* exception_names[] = {
   "Divide by zero",
   "Debug",
   "NMI interrupt",
   "Breakpoint",
   "Overflow",
   "BOUND range exceeded",
   "Invalid Opcode",
   "Device not available",
   "Double fault",
   "Coprocessor segment overrun",
   "Invalid TSS",
   "Segment not present",
   "Stack fault",
   "General protection",
   "Page fault",
   "Intel Reserved",
   "Floating point error",
   "Alignment check",
   "Machine check",
   "SIMD floating point"
};

void __attribute__ ((regparm(1))) vmm_excp_hdlr(int64_r0_ctx_t *ctx)
{
   debug(EXCP,
	 "- vmm %s exception\n"
	 " . cs:rip 0x%X:0x%X\n"
	 " . ss:rsp 0x%X:0x%X\n"
	 " . rflags 0x%X\n",
	 exception_names[ctx->nr.blow],
	 ctx->cs.raw, ctx->rip.raw,
	 ctx->ss.raw, ctx->rsp.raw,
	 ctx->rflags.raw
      );

   if(ctx->nr.low == PF_EXCP)
      debug(EXCP,
	    " . p:%d wr:%d us:%d id:%d addr 0x%X\n",
	    ctx->err.pf.p,
	    ctx->err.pf.wr,
	    ctx->err.pf.us,
	    ctx->err.pf.id,
	    get_cr2()
	 );
   else
      debug(EXCP,
	    " . ext:%d idt:%d ti:%d index:%d\n",
	    ctx->err.gp.ext,
	    ctx->err.gp.idt,
	    ctx->err.gp.ti,
	    ctx->err.gp.idx
	 );

   panic("vmm exception !\n");
}

int resolve_exception()
{
   int rc = gdb_excp_event(__exception_vector);

   if(rc == GDB_IGNORE)
      __inject_exception(__exception_vector, __exception_error, __exception_fault);

   return rc;
}
