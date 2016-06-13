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
#include <smi.h>
#include <asmutils.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

int __resolve_smi_1()
{
   eflags_reg_t saved, eflags;

   save_eflags( saved.raw );

   debug( SMI, "SMM: vm  flags.pf %d\n", __rflags.pf );

   eflags.raw = saved.raw;
   eflags.pf = 0;
   load_eflags( eflags.raw );

   debug( SMI, "SMM: vmm flags.pf %d\n", eflags.pf );

   smi_preempt();

   eflags.raw = get_eflags();

   debug( SMI, "RSM: vmm flags.pf %d\n", eflags.pf );

   load_eflags( saved.raw );
   return 1;
}

int __resolve_smi_2()
{
   uint32_t smm_stack[10];
   uint32_t *vm_ctx_loc = &smm_stack[1];

   /* prepare vm context to load before enter smm */
   memcpy( (void*)vm_ctx_loc, (void*)info->vm.cpu.gpr, sizeof(gpr_ctx_t) );
   smm_stack[9] = __rflags.low;

   asm volatile (
      "pushf               \n"
      "pusha               \n"
      "mov    %%esp, %0    \n"
      "mov    %%eax, %%esp \n"
      "popa                \n"
      "popf                \n"
      "nop;nop;stgi;nop;nop\n"
      "pushf               \n"
      "pusha               \n"
      "sub    $4, %%esp    \n"
      "pop    %%esp        \n"
      "popa                \n"
      "popf                \n"
      :
      :"m"(smm_stack[0]),"a"(vm_ctx_loc)
      :"memory"
      );

   /* commit vm context after rsm */
   memcpy( (void*)info->vm.cpu.gpr, (void*)vm_ctx_loc, sizeof(gpr_ctx_t) );
   __rflags.low = smm_stack[9];
   __post_access( __rflags );

   return 1;
}

int resolve_smi()
{
   return __resolve_smi_1();
   //return __resolve_smi_2();
}
