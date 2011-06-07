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
#include <svm_exit_msr.h>
#include <msr.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

static int __svm_vmexit_resolve_msr_efer(uint8_t wr)
{
   vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area;
   gpr64_ctx_t       *ctx = info->vm.cpu.gpr;
   uint32_t          efer_update = state->efer.eax ^ ctx->rax.low;

   if(wr)
   {
      if(efer_update & (AMD_EFER_LME|AMD_EFER_LMA|AMD_EFER_NXE))
	 __flush_tlb_glb();

      if(ctx->rax.low & AMD_EFER_SVME)
	 debug(SVM_MSR, "vm wants svm\n");

      state->efer.eax = ctx->rax.low | AMD_EFER_SVME;
      state->efer.edx = ctx->rdx.low;

      return MSR_SUCCESS;
   }

   ctx->rax.low = state->efer.eax & ~AMD_EFER_SVME;
   ctx->rdx.low = state->efer.edx;

   return MSR_SUCCESS;
}

static int __svm_vmexit_resolve_msr_gen(raw_msr_entry_t *msr, uint8_t wr)
{
   gpr64_ctx_t *ctx = info->vm.cpu.gpr;

   if(wr)
   {
      msr->eax = ctx->rax.low;
      msr->edx = ctx->rdx.low;
      return MSR_NATIVE;
   }

   return MSR_SUCCESS;
}

int __svm_vmexit_resolve_msr(uint8_t wr)
{
   vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area;

   switch(info->vm.cpu.gpr->rcx.low)
   {
   case IA32_SYSENTER_CS_MSR:
      return __svm_vmexit_resolve_msr_gen(&state->sysenter_cs, wr);
   case IA32_SYSENTER_ESP_MSR:
      return __svm_vmexit_resolve_msr_gen(&state->sysenter_esp, wr);
   case IA32_SYSENTER_EIP_MSR:
      return __svm_vmexit_resolve_msr_gen(&state->sysenter_eip, wr);
   case AMD_STAR_MSR:
      return __svm_vmexit_resolve_msr_gen(&state->star, wr);
   case AMD_LSTAR_MSR:
      return __svm_vmexit_resolve_msr_gen(&state->lstar, wr);
   case AMD_CSTAR_MSR:
      return __svm_vmexit_resolve_msr_gen(&state->cstar, wr);
   case AMD_SFMASK_MSR:
      return __svm_vmexit_resolve_msr_gen(&state->sfmask, wr);
   case AMD_KERNEL_GS_BASE_MSR:
      return __svm_vmexit_resolve_msr_gen(&state->kernel_gs_base, wr);
   case IA32_PAT_MSR:
      return __svm_vmexit_resolve_msr_gen(&state->g_pat, wr);
   case AMD_EFER_MSR:
      return __svm_vmexit_resolve_msr_efer(wr);
   default:
      return MSR_FAIL;
   }
}

int svm_vmexit_resolve_msr()
{
   return resolve_msr(info->vm.cpu.vmc->vm_vmcb.ctrls_area.exit_info_1.low);
}
