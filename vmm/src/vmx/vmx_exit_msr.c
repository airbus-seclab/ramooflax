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
#include <vmx_exit_msr.h>
#include <msr.h>
#include <mtrr.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int __vmx_vmexit_resolve_msr_mtrr_def(uint8_t wr)
{
   if(wr)
   {
      ia32_mtrr_def_t update;

      update.low  = info->vm.mtrr_def.low  ^ info->vm.cpu.gpr->rax.low;
      update.high = info->vm.mtrr_def.high ^ info->vm.cpu.gpr->rdx.low;

      info->vm.mtrr_def.low  = info->vm.cpu.gpr->rax.low;
      info->vm.mtrr_def.high = info->vm.cpu.gpr->rdx.low;

      if(update.e)
	 vmx_ept_remap();
   }
   else
   {
      info->vm.cpu.gpr->rax.low = info->vm.mtrr_def.low;
      info->vm.cpu.gpr->rdx.low = info->vm.mtrr_def.high;
   }

   return VM_DONE;
}

static int __vmx_vmexit_resolve_msr_efer(uint8_t wr)
{
   if(wr)
   {
      ia32_efer_msr_t update;

      update.low = info->vm.efer.low ^ info->vm.cpu.gpr->rax.low;

      info->vm.efer.low  = info->vm.cpu.gpr->rax.low;
      info->vm.efer.high = info->vm.cpu.gpr->rdx.low;

      vm_state.ia32_efer.low  = info->vm.efer.low;
      vm_state.ia32_efer.high = info->vm.efer.high;

      vmcs_read(vm_entry_ctrls.entry);
      vm_state.ia32_efer.lme = vm_state.ia32_efer.lma = vm_entry_ctrls.entry.ia32e;
      vmcs_dirty(vm_state.ia32_efer);

      if(info->vm.efer.lma && !info->vm.efer.lme)
	 info->vm.efer.lma = 0;

      if(update.lme && __cr0.pg)
      {
	 debug(VMX_MSR, "modifying LME while paging-on #GP\n");
	 __inject_exception(GP_EXCP, 0, 0);
	 return VM_FAULT;
      }
   }
   else
   {
      info->vm.cpu.gpr->rax.low = info->vm.efer.low;
      info->vm.cpu.gpr->rdx.low = info->vm.efer.high;
   }

   return VM_DONE;
}

#ifdef CONFIG_MSR_PROXY
static int __vmx_vmexit_resolve_msr_sysenter_cs(uint8_t wr)
{
   if(wr)
   {
      vm_state.ia32_sysenter_cs.raw = info->vm.cpu.gpr->rax.low;
      vmcs_dirty(vm_state.ia32_sysenter_cs);
   }
   else
   {
      vmcs_read(vm_state.ia32_sysenter_cs);
      info->vm.cpu.gpr->rax.low = vm_state.ia32_sysenter_cs.raw;
   }

   return VM_DONE;
}

static int __vmx_vmexit_resolve_msr_sysenter_esp(uint8_t wr)
{
   if(wr)
   {
      vm_state.ia32_sysenter_esp.low  = info->vm.cpu.gpr->rax.low;
      vm_state.ia32_sysenter_esp.high = info->vm.cpu.gpr->rdx.low;
      vmcs_dirty(vm_state.ia32_sysenter_esp);
   }
   else
   {
      vmcs_read(vm_state.ia32_sysenter_esp);
      info->vm.cpu.gpr->rax.low = vm_state.ia32_sysenter_esp.low;
      info->vm.cpu.gpr->rdx.low = vm_state.ia32_sysenter_esp.high;
   }

   return VM_DONE;
}

static int __vmx_vmexit_resolve_msr_sysenter_eip(uint8_t wr)
{
   if(wr)
   {
      vm_state.ia32_sysenter_eip.low  = info->vm.cpu.gpr->rax.low;
      vm_state.ia32_sysenter_eip.high = info->vm.cpu.gpr->rdx.low;
      vmcs_dirty(vm_state.ia32_sysenter_eip);
   }
   else
   {
      vmcs_read(vm_state.ia32_sysenter_eip);
      info->vm.cpu.gpr->rax.low = vm_state.ia32_sysenter_eip.low;
      info->vm.cpu.gpr->rdx.low = vm_state.ia32_sysenter_eip.high;
   }

   return VM_DONE;
}

static int __vmx_vmexit_resolve_msr_pat(uint8_t wr)
{
   if(wr)
   {
      vm_state.ia32_pat.low  = info->vm.cpu.gpr->rax.low;
      vm_state.ia32_pat.high = info->vm.cpu.gpr->rdx.low;
      vmcs_dirty(vm_state.ia32_pat);
   }
   else
   {
      vmcs_read(vm_state.ia32_pat);
      info->vm.cpu.gpr->rax.low = vm_state.ia32_pat.low;
      info->vm.cpu.gpr->rdx.low = vm_state.ia32_pat.high;
   }

   return VM_DONE;
}

static int __vmx_vmexit_resolve_msr_dbgctl(uint8_t wr)
{
   if(wr)
   {
      vm_state.ia32_dbgctl.low  = info->vm.cpu.gpr->rax.low;
      vm_state.ia32_dbgctl.high = info->vm.cpu.gpr->rdx.low;
      vmcs_dirty(vm_state.ia32_dbgctl);
   }
   else
   {
      vmcs_read(vm_state.ia32_dbgctl);
      info->vm.cpu.gpr->rax.low = vm_state.ia32_dbgctl.low;
      info->vm.cpu.gpr->rdx.low = vm_state.ia32_dbgctl.high;
   }

   return VM_DONE;
}

static int __vmx_vmexit_resolve_msr_perf(uint8_t wr)
{
   if(wr)
   {
      vm_state.ia32_perf.low  = info->vm.cpu.gpr->rax.low;
      vm_state.ia32_perf.high = info->vm.cpu.gpr->rdx.low;
      vmcs_dirty(vm_state.ia32_perf);
   }
   else
   {
      vmcs_read(vm_state.ia32_perf);
      info->vm.cpu.gpr->rax.low = vm_state.ia32_perf.low;
      info->vm.cpu.gpr->rdx.low = vm_state.ia32_perf.high;
   }

   return VM_DONE;
}
#endif

int __vmx_vmexit_resolve_msr(uint8_t wr)
{
   switch(info->vm.cpu.gpr->rcx.low)
   {
   case IA32_MTRR_DEF_TYPE:
      return __vmx_vmexit_resolve_msr_mtrr_def(wr);
   case IA32_EFER_MSR:
      return __vmx_vmexit_resolve_msr_efer(wr);

#ifdef CONFIG_MSR_PROXY
   case IA32_SYSENTER_CS_MSR:
      return __vmx_vmexit_resolve_msr_sysenter_cs(wr);
   case IA32_SYSENTER_ESP_MSR:
      return __vmx_vmexit_resolve_msr_sysenter_esp(wr);
   case IA32_SYSENTER_EIP_MSR:
      return __vmx_vmexit_resolve_msr_sysenter_eip(wr);
   case IA32_PAT_MSR:
      return __vmx_vmexit_resolve_msr_pat(wr);
   case IA32_DBG_CTL_MSR:
      return __vmx_vmexit_resolve_msr_dbgctl(wr);
   case IA32_PERF_GLB_CTL_MSR:
      return __vmx_vmexit_resolve_msr_perf(wr);
#endif

   default:
      return VM_NATIVE;
   }
}

int vmx_vmexit_resolve_msr_rd()
{
   return resolve_msr(0);
}

int vmx_vmexit_resolve_msr_wr()
{
   return resolve_msr(1);
}
