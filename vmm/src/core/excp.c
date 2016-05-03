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
#include <ctrl.h>
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

static int __resolve_exception(uint32_t vector, uint32_t error, uint64_t fault)
{
   int rc;

   info->vm.cpu.fault.excp.err.raw = error;

   if((rc=ctrl_evt_excp(vector)) == VM_IGNORE)
   {
      debug(EXCP, "ignored excp, injecting\n");
      __inject_exception(vector, error, fault);
   }

   return rc;
}

int resolve_exception()
{
   if(__exception_vector == PF_EXCP)
   {
      __cr2.raw = __exception_fault;
      __post_access(__cr2);
   }

   return __resolve_exception(__exception_vector,
			      __exception_error.raw,
			      __exception_fault);
}

void __regparm__(1) vmm_excp_hdlr(int64_r0_ctx_t *ctx)
{
   debug(EXCP,
	 "\nvmm native exception -= %s =-\n"
	 " . excp #%d error 0x%X\n"
	 " . cs:rip 0x%X:0x%X\n"
	 " . ss:rsp 0x%X:0x%X\n"
	 " . rflags 0x%X\n"
	 "\n- general registers\n"
	 "rax     : 0x%X\n"
	 "rcx     : 0x%X\n"
	 "rdx     : 0x%X\n"
	 "rbx     : 0x%X\n"
	 "rsp     : 0x%X\n"
	 "rbp     : 0x%X\n"
	 "rsi     : 0x%X\n"
	 "rdi     : 0x%X\n"
	 "r08     : 0x%X\n"
	 "r09     : 0x%X\n"
	 "r10     : 0x%X\n"
	 "r11     : 0x%X\n"
	 "r12     : 0x%X\n"
	 "r13     : 0x%X\n"
	 "r14     : 0x%X\n"
	 "r15     : 0x%X\n"
	 ,exception_names[ctx->nr.blow]
	 ,ctx->nr.blow, ctx->err.raw
	 ,ctx->cs.raw,  ctx->rip.raw
	 ,ctx->ss.raw,  ctx->rsp.raw
	 ,ctx->rflags.raw
	 ,ctx->gpr.rax.raw
	 ,ctx->gpr.rcx.raw
	 ,ctx->gpr.rdx.raw
	 ,ctx->gpr.rbx.raw
	 ,ctx->gpr.rsp.raw
	 ,ctx->gpr.rbp.raw
	 ,ctx->gpr.rsi.raw
	 ,ctx->gpr.rdi.raw
	 ,ctx->gpr.r8.raw
	 ,ctx->gpr.r9.raw
	 ,ctx->gpr.r10.raw
	 ,ctx->gpr.r11.raw
	 ,ctx->gpr.r12.raw
	 ,ctx->gpr.r13.raw
	 ,ctx->gpr.r14.raw
	 ,ctx->gpr.r15.raw);

   switch(ctx->nr.blow)
   {
   case NMI_EXCP:
      debug(EXCP, "#NMI (ignored)\n");
      return;

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
	    ,ctx->err.sl.ext
	    ,ctx->err.sl.idt
	    ,ctx->err.sl.ti
	    ,ctx->err.sl.idx);
      break;
   }

   panic("vmm exception !\n");
}
