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
#include <vmx_exit_dt.h>
#include <vmx_vmcs_acc.h>
#include <emulate.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

static int __vmx_vmexit_lgdt(dt_reg_t *dt_reg)
{
   if(vmx_cpl)
      return VM_FAIL;

   debug(VMX_DT, "lgdt 0x%X:0x%x\n", dt_reg->base.raw, dt_reg->limit);

   vm_state.gdtr.base.raw = dt_reg->base.raw;
   vm_state.gdtr.limit.wlow = dt_reg->limit;
   vmcs_dirty(vm_state.gdtr.base);
   vmcs_dirty(vm_state.gdtr.limit);
   return VM_DONE;
}

static int __vmx_vmexit_lidt(dt_reg_t *dt_reg)
{
   if(vmx_cpl)
      return VM_FAIL;

   debug(VMX_DT, "lidt 0x%X:0x%x\n", dt_reg->base.raw, dt_reg->limit);

   if(info->vm.idt_limit_saved != dt_reg->limit)
      info->vm.idt_limit_saved = dt_reg->limit;

   vm_state.idtr.base.raw = dt_reg->base.raw;
   vmcs_dirty(vm_state.idtr.base);
   return VM_DONE;
}

static int __vmx_vmexit_sgdt(dt_reg_t *dt_reg)
{
   vmcs_read(vm_state.gdtr.base);
   vmcs_read(vm_state.gdtr.limit);

   dt_reg->base.raw = vm_state.gdtr.base.raw;
   dt_reg->limit = vm_state.gdtr.limit.wlow;

   debug(VMX_DT, "sgdt\n");
   return VM_DONE;
}

static int __vmx_vmexit_sidt(dt_reg_t *dt_reg)
{
   vmcs_read(vm_state.idtr.base);

   dt_reg->base.raw = vm_state.idtr.base.raw;
   dt_reg->limit = info->vm.idt_limit_saved;

   debug(VMX_DT, "sidt\n");
   return VM_DONE;
}

int vmx_vmexit_resolve_dt()
{
   vmcs_exit_info_insn_dt_t *dt_insn;
   offset_t                  dt_addr;
   dt_reg_t                  dt_reg;
   raw64_t                   disp;
   uint64_t                  addr_msk, op_msk;
   int                       rc, sz, mode;

   if(!__rmode())
   {
      debug(VMX_DT, "DT intercept only while in real mode\n");
      return VM_FAIL;
   }

   vmcs_read(vm_exit_info.insn_info);
   vmcs_read(vm_exit_info.qualification);

   dt_insn   = &vm_exit_info.insn_info.dt;
   dt_addr   = 0;
   disp.sraw = vm_exit_info.qualification.sraw;
   addr_msk  = (1ULL<<(16*(1<<dt_insn->addr))) - 1;

   switch(dt_insn->seg)
   {
   case VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_ES:
      vmcs_read(vm_state.es.base);
      dt_addr += vm_state.es.base.raw;
      break;
   case VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_CS:
      vmcs_read(vm_state.cs.base);
      dt_addr += vm_state.cs.base.raw;
      break;
   case VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_SS:
      vmcs_read(vm_state.ss.base);
      dt_addr += vm_state.ss.base.raw;
      break;
   case VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_DS:
      vmcs_read(vm_state.ds.base);
      dt_addr += vm_state.ds.base.raw;
      break;
   case VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_FS:
      vmcs_read(vm_state.fs.base);
      dt_addr += vm_state.fs.base.raw;
      break;
   case VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_GS:
      vmcs_read(vm_state.gs.base);
      dt_addr += vm_state.gs.base.raw;
      break;
   }

   /* XXX: compute offset alone and check against segment limit */
   if(!dt_insn->no_base)
   {
      int reg = GPR64_RAX - (dt_insn->base & GPR64_RAX);
      dt_addr += info->vm.cpu.gpr->raw[reg].raw & addr_msk;
   }

   if(!dt_insn->no_idx)
   {
      int      reg = GPR64_RAX - (dt_insn->idx & GPR64_RAX);
      uint64_t val = info->vm.cpu.gpr->raw[reg].raw & addr_msk;

      if(dt_insn->scale)
	 val *= (1ULL<<dt_insn->scale);

      dt_addr += val;
   }

   dt_addr += (disp.sraw & addr_msk);
   mode = cpu_addr_sz();

   if(mode == 64)
   {
      op_msk = -1ULL;
      sz = 10;
   }
   else if(dt_insn->op == VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_OP_SZ_16)
   {
      op_msk = (1ULL<<24) - 1;
      sz = 6;
   }
   else
   {
      op_msk = (1ULL<<32) - 1;
      sz = 6;
   }

   debug(VMX_DT, "dt op @ 0x%X\n", dt_addr);

   if(dt_insn->type < VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_TYPE_LGDT)
   {
      if(dt_insn->type == VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_TYPE_SGDT)
	 rc = __vmx_vmexit_sgdt(&dt_reg);
      else
	 rc = __vmx_vmexit_sidt(&dt_reg);

      dt_reg.base.raw &= op_msk;
      if(vm_write_mem(dt_addr, (uint8_t*)&dt_reg, sz) != VM_DONE)
      {
	 debug(VMX_DT, "could not write vm mem @0x%X\n", dt_addr);
	 return VM_FAIL;
      }
   }
   else
   {
      if(vm_read_mem(dt_addr, (uint8_t*)&dt_reg, sz) != VM_DONE)
      {
	 debug(VMX_DT, "could not read vm mem @0x%X\n", dt_addr);
	 return VM_FAIL;
      }

      dt_reg.base.raw &= op_msk;

      if(dt_insn->type == VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_TYPE_LGDT)
	 rc = __vmx_vmexit_lgdt(&dt_reg);
      else
	 rc = __vmx_vmexit_lidt(&dt_reg);
   }

   vmcs_read(vm_exit_info.insn_len);
   return emulate_done(rc, vm_exit_info.insn_len.raw);
}
