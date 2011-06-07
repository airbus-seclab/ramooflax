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
#ifndef __SVM_VM_H__
#define __SVM_VM_H__

#include <types.h>

#define __cr0                   (info->vm.cpu.vmc->vm_vmcb.state_area.cr0)
#define __cr2                   (info->vm.cpu.vmc->vm_vmcb.state_area.cr2)
#define __cr3                   (info->vm.cpu.vmc->vm_vmcb.state_area.cr3)
#define __cr4                   (info->vm.cpu.vmc->vm_vmcb.state_area.cr4)
#define __rip                   (info->vm.cpu.vmc->vm_vmcb.state_area.rip)
#define __rflags                (info->vm.cpu.vmc->vm_vmcb.state_area.rflags)

#define __dr6                   (info->vm.cpu.vmc->vm_vmcb.state_area.dr6)
#define __dr7                   (info->vm.cpu.vmc->vm_vmcb.state_area.dr7)

#define __cs                    (info->vm.cpu.vmc->vm_vmcb.state_area.cs)
#define __ds                    (info->vm.cpu.vmc->vm_vmcb.state_area.ds)
#define __es                    (info->vm.cpu.vmc->vm_vmcb.state_area.es)
#define __fs                    (info->vm.cpu.vmc->vm_vmcb.state_area.fs)
#define __gs                    (info->vm.cpu.vmc->vm_vmcb.state_area.gs)
#define __ss                    (info->vm.cpu.vmc->vm_vmcb.state_area.ss)

#define __vmexit_on_insn()      __svm_vmexit_on_insn()

#define __cpl                   (info->vm.cpu.vmc->vm_vmcb.state_area.cpl)
#define __gdtr                  (info->vm.cpu.vmc->vm_vmcb.state_area.gdtr)
#define __idtr                  (info->vm.cpu.vmc->vm_vmcb.state_area.idtr)
#define __ldtr                  (info->vm.cpu.vmc->vm_vmcb.state_area.ldtr)
#define __tr                    (info->vm.cpu.vmc->vm_vmcb.state_area.tr)
#define __exception_bitmap      (info->vm.cpu.vmc->vm_vmcb.ctrls_area.exception_bitmap)
#define __cr_rd_bitmap          (info->vm.cpu.vmc->vm_vmcb.ctrls_area.cr_read_bitmap)
#define __cr_wr_bitmap          (info->vm.cpu.vmc->vm_vmcb.ctrls_area.cr_write_bitmap)
#define __efer                  (info->vm.cpu.vmc->vm_vmcb.state_area.efer)
#define __dbgctl                (info->vm.cpu.vmc->vm_vmcb.state_area.dbgctl)

#define __sysenter_cs           (info->vm.cpu.vmc->vm_vmcb.state_area.sysenter_cs)
#define __sysenter_eip          (info->vm.cpu.vmc->vm_vmcb.state_area.sysenter_eip)
#define __sysenter_esp          (info->vm.cpu.vmc->vm_vmcb.state_area.sysenter_esp)

#define _xx_lmode()             (__efer.lma)
#define _xx_pmode()             (__cr0.pe)

/*
** long mode: 64 bits mode and compatibility mode 32:16
*/
#define __lmode64()             (_xx_lmode() &&  __cs.attributes.l)
#define __lmode32()             (_xx_lmode() && !__cs.attributes.l &&  __cs.attributes.d)
#define __lmode16()             (_xx_lmode() && !__cs.attributes.l && !__cs.attributes.d)

/*
** legacy mode: real mode, protected mode 32:16, virtual8086 mode
*/
#define __pmode32()             (_xx_pmode() && !__rflags.vm &&  __cs.attributes.d)
#define __pmode16()             (_xx_pmode() && !__rflags.vm && !__cs.attributes.d)

#define __rmode()               (!_xx_lmode() && !_xx_pmode())
#define __v86mode()             (!_xx_lmode() &&  _xx_pmode() && __rflags.vm)
#define __legacy32()            (!_xx_lmode() &&  __pmode32())
#define __legacy16()            (!_xx_lmode() &&  __pmode16())

#define __paging()              (__cr0.pg)

/*
** segment settings
*/
#define SVM_CODE_32_R0_ATTR     0xbc9b
#define SVM_CODE_16_R0_ATTR     0xb09b
#define SVM_CODE_16_R0_CO_ATTR  0xb09f
#define SVM_CODE_16_R3_ATTR     0xb0fb

#define SVM_DATA_32_R0_ATTR     0xbc93
#define SVM_DATA_16_R0_ATTR     0xb093
#define SVM_DATA_16_R3_ATTR     0xb0f3

#define __set_code16r0_desc(_attr_)       (_attr_.raw = SVM_CODE_16_R0_ATTR)
#define __set_data16r0_desc(_attr_)       (_attr_.raw = SVM_DATA_16_R0_ATTR)
#define __set_data16r3_desc(_attr_)       (_attr_.raw = SVM_DATA_16_R3_ATTR)

#define __pre_access(_field_)                     ({})
#define __post_access(_field_)                    ({})
#define __post_access_restrictive(_field_,_idx_)  ({})

/*
** Events
*/
#define __update_exception_mask()					\
   ({__exception_bitmap.raw =						\
	 info->vm.cpu.dflt_excp | info->vmm.ctrl.usr.excp | info->vmm.ctrl.dbg.excp;})

#define __update_cr_read_mask()						\
   ({__cr_rd_bitmap =							\
	 VMCB_CR_R_BITMAP | info->vmm.ctrl.usr.cr_rd;})

#define __update_cr_write_mask()					\
   ({__cr_wr_bitmap =							\
	 VMCB_CR_W_BITMAP | info->vmm.ctrl.usr.cr_wr;})

#define __allow_dr_access()						\
   ({									\
      info->vm.cpu.vmc->vm_vmcb.ctrls_area.dr_read_bitmap = 0;		\
      info->vm.cpu.vmc->vm_vmcb.ctrls_area.dr_write_bitmap = 0;		\
   })

#define __deny_dr_access()						\
   ({									\
      info->vm.cpu.vmc->vm_vmcb.ctrls_area.dr_read_bitmap = -1;		\
      info->vm.cpu.vmc->vm_vmcb.ctrls_area.dr_write_bitmap = -1;	\
   })

#define __exit_reason					\
   (info->vm.cpu.vmc->vm_vmcb.ctrls_area.exit_code.low)

#define __vmexit_on_excp()                __svm_vmexit_on_excp()

#define __exception_vector				\
   (info->vm.cpu.vmc->vm_vmcb.ctrls_area.exit_code.low - SVM_VMEXIT_EXCP_START)

#define __exception_error				\
   info->vm.cpu.vmc->vm_vmcb.ctrls_area.exit_info_1.low

#define __exception_fault				\
   info->vm.cpu.vmc->vm_vmcb.ctrls_area.exit_info_2.low

#define __injecting_exception()				\
   (info->vm.cpu.vmc->vm_vmcb.ctrls_area.event_injection.v?1:0)

#define __svm_prepare_event_injection(_ev, _type, _vector)		\
   ({									\
      _ev->raw    = 0ULL;						\
      _ev->vector = _vector;						\
      _ev->type   = _type;						\
      _ev->v      = 1;							\
   })

#define __inject_intn(n)            __svm_vmexit_inject_intn(n)
#define __inject_exception(a,b,c)   __svm_vmexit_inject_exception(a,b,c)

#define __ncr3                   (info->vm.cpu.vmc->vm_vmcb.ctrls_area.ncr3)
#define __asid_tlb_supported()   svm_flush_asid_supported()

#define __flush_tlb_glb()						\
   ({									\
      vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;	\
      if(info->vmm.cpu.skillz.asid_tlb)					\
	 ctrls->tlb_ctrl.tlb_control = VMCB_TLB_CTL_FLUSH_GUEST_ALL;	\
      else								\
	 ctrls->tlb_ctrl.tlb_control = VMCB_TLB_CTL_FLUSH_ALL;		\
   })

#define __flush_tlb()							\
   ({									\
      vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;	\
      if(info->vmm.cpu.skillz.asid_tlb)					\
	 ctrls->tlb_ctrl.tlb_control = VMCB_TLB_CTL_FLUSH_GUEST;	\
      else								\
	 ctrls->tlb_ctrl.tlb_control = VMCB_TLB_CTL_FLUSH_ALL;		\
   })

#define __allow_pushf()							\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.pushf=0;})

#define __deny_pushf()							\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.pushf=1;})

#define __allow_popf()							\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.popf=0;})

#define __deny_popf()							\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.popf=1;})

#define __allow_iret()							\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.iret=0;})

#define __deny_iret()							\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.iret=1;})

#define __allow_soft_int()						\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.intn=0;})

#define __deny_soft_int()						\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.intn=1;})

#define __allow_hrdw_int()						\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.intr=0;})

#define __deny_hrdw_int()						\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.sys_insn_bitmap.intr=1;})

#define __allow_icebp()							\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.vmm_insn_bitmap.icebp=0;})

#define __deny_icebp()							\
   ({info->vm.cpu.vmc->vm_vmcb.ctrls_area.vmm_insn_bitmap.icebp=1;})

#define __inject_intr(a)	          __svm_vmexit_inject_interrupt(a)

#define __interrupt_shadow				\
   (info->vm.cpu.vmc->vm_vmcb.ctrls_area.int_shadow)

#define __setup_iwe(_on_,_nr_)					\
   __svm_vmexit_setup_interrupt_window_exiting(_on_,_nr_)

#define __interrupts_on()      (__rflags.IF)
#define __safe_interrupts_on() (__interrupts_on() && !__interrupt_shadow.low)
#define __iwe_on()             (info->vm.cpu.vmc->vm_vmcb.ctrls_area.int_ctrl.v_irq)

#define __resolve_msr_arch(_wr_)          __svm_vmexit_resolve_msr(_wr_)
#define __resolve_cpuid_arch(_idx_)       __svm_vmexit_resolve_cpuid(_idx_)
#define __allow_io(_p_)                   svm_allow_io(info->vm.cpu.vmc,_p_)
#define __allow_io_range(_p1,_p2)         svm_allow_io_range(info->vm.cpu.vmc,_p1,_p2)
#define __deny_io(_p_)                    svm_deny_io(info->vm.cpu.vmc,_p_)
#define __deny_io_range(_p1,_p2)          svm_deny_io_range(info->vm.cpu.vmc,_p1,_p2)

#define __enable_lbr()						\
   ({info->vm.cpu.vmc->vm_vmcb.state_area.dbgctl.lbr=1;})

#define __disable_lbr()						\
   ({info->vm.cpu.vmc->vm_vmcb.state_area.dbgctl.lbr=0;})

#define __lbr_from()						\
   ({info->vm.cpu.vmc->vm_vmcb.state_area.lbr_from.raw;})

#define __lbr_to()					\
   ({info->vm.cpu.vmc->vm_vmcb.state_area.lbr_to.raw;})

#define __lbr_from_excp()					\
   ({info->vm.cpu.vmc->vm_vmcb.state_area.lbr_from_excp.raw;})

#define __lbr_to_excp()						\
   ({info->vm.cpu.vmc->vm_vmcb.state_area.lbr_to_excp.raw;})

/*
** Functions
*/
#ifdef __INIT__
void  svm_vm_init();
#endif

#endif
