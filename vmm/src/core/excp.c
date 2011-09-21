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

void vmm_excp_mce()
{
   ia32_mcg_cap_t    cap;
   ia32_mci_status_t mci_sts;
   uint16_t          i;
   uint32_t          c=0,d;

   cpuid_features(c,d);

   if(! (d & (CPUID_EDX_FEAT_MCE|CPUID_EDX_FEAT_MCA)))
   {
      debug(EXCP, "MCE/MCA unsupported\n");
      return;
   }

   rd_msr_ia32_mcg_cap(cap);
   debug(EXCP, "MCE_CAP 0x%X\n", cap.raw);

   for(i=0 ; i<cap.count; i++)
   {
      rd_msr_ia32_mci_status(mci_sts, i);
      debug(EXCP, "MC%d_STS 0x%X\n", i, mci_sts.raw);
   }
}

void __regparm__(1) vmm_excp_hdlr(int64_r0_ctx_t *ctx)
{
   debug(EXCP,
	 "vmm %s exception\n"
	 " . excp #%d error 0x%X\n"
	 " . cs:rip 0x%X:0x%X\n"
	 " . ss:rsp 0x%X:0x%X\n"
	 " . rflags 0x%X\n"
	 ,exception_names[ctx->nr.blow]
	 ,ctx->nr.blow, ctx->err.raw
	 ,ctx->cs.raw, ctx->rip.raw
	 ,ctx->ss.raw, ctx->rsp.raw
	 ,ctx->rflags.raw);

   switch(ctx->nr.blow)
   {
   case MC_EXCP:
      vmm_excp_mce();
      break;
   case PF_EXCP:
      debug(EXCP,
	    "#PF details: p:%d wr:%d us:%d id:%d addr 0x%X\n"
	    ,ctx->err.pf.p
	    ,ctx->err.pf.wr
	    ,ctx->err.pf.us
	    ,ctx->err.pf.id
	    ,get_cr2());
      break;
   case GP_EXCP:
      debug(EXCP,
	    "#GP details: ext:%d idt:%d ti:%d index:%d\n"
	    ,ctx->err.gp.ext
	    ,ctx->err.gp.idt
	    ,ctx->err.gp.ti
	    ,ctx->err.gp.idx);
      break;
   }

   panic("vmm exception !\n");
}

int resolve_exception()
{
   int rc;

   if(__exception_vector == PF_EXCP)
   {
      __cr2.raw = __exception_fault;
      __post_access(__cr2);
   }

   rc = gdb_excp_event(__exception_vector);

   if(rc == GDB_IGNORE)
      __inject_exception(__exception_vector, __exception_error, __exception_fault);

   return rc;
}
