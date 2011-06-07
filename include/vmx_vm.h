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
#ifndef __VMX_VM_H__
#define __VMX_VM_H__

#include <types.h>

/**** XXX tout est a revoir ******/

#define __cs                    (info->vm.vmcs.guest_state.cs)
#define __ds                    (info->vm.vmcs.guest_state.ds)
#define __es                    (info->vm.vmcs.guest_state.es)
#define __fs                    (info->vm.vmcs.guest_state.fs)
#define __gs                    (info->vm.vmcs.guest_state.gs)
#define __ss                    (info->vm.vmcs.guest_state.ss)
#define __rflags                (info->vm.vmcs.guest_state.rflags)
#define __rip                   (info->vm.vmcs.guest_state.rip)
#define __rpl                   (info->vm.vmcs.guest_state.cs.selector.rpl)
#define __cpl                   (info->vm.vmcs.guest_state.cs.selector.rpl)

#define __gdtr                  (info->vm.vmcs.guest_state.gdtr)
#define __idtr                  (info->vm.vmcs.guest_state.idtr)
#define __tr                    (info->vm.vmcs.guest_state.tr)

#define __cr0                   (info->vm.vmcs.guest_state.cr0)
#define __cr3                   (info->vm.vmcs.guest_state.cr3)
#define __cr4                   (info->vm.vmcs.guest_state.cr4)

#define __vmexit_on_insn()      __vmx_vmexit_on_insn()

#define __cr0_shadow            (info->vm.vmcs.controls.exec_ctrls.cr0_read_shadow)
#define __cr4_shadow            (info->vm.vmcs.controls.exec_ctrls.cr4_read_shadow)
#define __cr3_shadow            (info->vm.cr3_shadow)
#define __g_pgd                 (__cr3_shadow.low)

#define __exception_bitmap      (info->vm.vmcs.controls.exec_ctrls.excp_bitmap)
#define __resolve_pf()          __vmx_vmexit_resolve_pf()

#define __update_asid()         __vmx_update_asid()

#define __force_ring0()
#define __force_ring3()

#define __inject_exception(a,b,c) __vmx_vmexit_inject_exception(a,b,c)
#define __inject_interrupt(_i_)   __vmx_vmexit_inject_interrupt(_i_)
#define __setup_iwe(_on_,_nr_)    __vmx_vmexit_setup_interrupt_window_exiting(_on_,_nr_)
#define __iwe_on()                (info->vm.vmcs.controls.exec_ctrls.proc.iwe)
#define __pending_irq             (info->vm.last_pending)
#define __interrupts_on()         (__rflags.IF)
/* XXX: interrupt shadow, blocking by STI, mov SS ? */
#define __interrupt_shadow        (info->vm.int_shadow)
#define __safe_interrupts_on()    (__interrupts_on() && ! __interrupt_shadow.low)
#define __blocking_by_mov_ss()					\
   ({								\
      vmcs_read( info->vm.vmcs.guest_state.int_state );		\
      info->vm.vmcs.guest_state.int_state.mss = 1;		\
      vmcs_dirty( info->vm.vmcs.guest_state.int_state );	\
   })

#define __mov_from_cr(_cr_)     __vmx_vmexit_mov_from_cr(_cr_)

#define __pse()                 ((__cr4_shadow.low & CR4_PSE)?1:0)
#define __pae()                 ((__cr4_shadow.low & CR4_PAE)?1:0)
#define __paging()              ((__cr0_shadow.low & CR0_PG)?1:0)
#define __rmode()               ((__cr0_shadow.low & CR0_PE)?0:1)
#define __wp()                  ((__cr0.low & CR0_WP)?1:0)

#define __allow_exception(_e_)  __vmx_allow_exception(info->vm.vmcs,_e_)
#define __deny_exception(_e_)   __vmx_deny_exception(info->vm.vmcs,_e_)

#define __pre_access(_field_)               vmcs_read(_field_)
#define __post_access(_field_)              vmcs_dirty(_field_)
#define __post_access_restrictive(_field_,_idx_)	\
   ({							\
      cr##_idx_##_reg_t f0,f1;				\
      read_msr_vmx_cr##_idx_##_fixed0(f0);		\
      read_msr_vmx_cr##_idx_##_fixed1(f1);		\
      _field_.low &= f1.low;				\
      _field_.low |= f0.low;				\
      vmcs_dirty(_field_);				\
   })

#define __resolve_msr_arch(_wr_)            __vmx_vmexit_resolve_msr(_wr_)
#define __resolve_cpuid_arch(_idx_)         __vmx_vmexit_resolve_cpuid(_idx_)
#define __allow_io(_p_)                     vmx_allow_io(info->vm.cpu.vmc,_p_)
#define __allow_io_range(_p1_,_p2_)         vmx_allow_io_range(info->vm.cpu.vmc,_p1_,_p2_)
#define __deny_io_range(_p1_,_p2_)          vmx_deny_io_range(info->vm.cpu.vmc,_p1_,_p2_)
#define __deny_io(_p_)                      vmx_deny_io(info->vm.cpu.vmc,_p_)
#define __release_irq()                     vmx_release_irq()


#define VMX_CODE_32_R0_ATTR     0xc09b
#define VMX_CODE_16_R0_ATTR     0x809b
#define VMX_CODE_16_R0_CO_ATTR  0x809f
#define VMX_CODE_16_R1_CO_ATTR  0x80bf
#define VMX_CODE_16_R3_ATTR     0x80fb

#define __set_code16r0co_desc(_attr_)       (_attr_.raw = VMX_CODE_16_R0_CO_ATTR)

#define VMX_DATA_32_R0_ATTR     0xc093
#define VMX_DATA_16_R0_ATTR     0x8093
#define VMX_DATA_16_R1_ATTR     0x80b3
#define VMX_DATA_16_R3_ATTR     0x80f3

#define __set_data16r0_desc(_attr_)         (_attr_.raw = VMX_DATA_16_R0_ATTR)
#define __set_data16r3_desc(_attr_)         (_attr_.raw = VMX_DATA_16_R3_ATTR)

#define VMX_TSS_32_ATTR         0x8b

#define __set_tss32_desc(_attr_)            (_attr_.raw = VMX_TSS_32_ATTR)

/*
** Functions
*/
struct information_data;

void vmx_vm_init(struct information_data*);

#endif
