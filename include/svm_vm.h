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

#define __state                 info->vm.cpu.vmc->vm_vmcb.state_area
#define __ctrls                 info->vm.cpu.vmc->vm_vmcb.ctrls_area

/*
** System registers
*/
#define __cr0                   (__state.cr0)
#define __cr2                   (__state.cr2)
#define __cr3                   (__state.cr3)
#define __cr4                   (__state.cr4)
#define __rip                   (__state.rip)
#define __rflags                (__state.rflags)

#define __dr6                   (__state.dr6)
#define __dr7                   (__state.dr7)

#define __cs                    (__state.cs)
#define __ds                    (__state.ds)
#define __es                    (__state.es)
#define __fs                    (__state.fs)
#define __gs                    (__state.gs)
#define __ss                    (__state.ss)

#define __cpl                   (__state.cpl)
#define __gdtr                  (__state.gdtr)
#define __idtr                  (__state.idtr)
#define __ldtr                  (__state.ldtr)
#define __tr                    (__state.tr)

#define __efer                  (__state.efer)
#define __dbgctl                (__state.dbgctl)
#define __sysenter_cs           (__state.sysenter_cs)
#define __sysenter_eip          (__state.sysenter_eip)
#define __sysenter_esp          (__state.sysenter_esp)

#include <cpu_modes.h>

/*
** Segmentation
*/
#define SVM_CODE_32_R0_ATTR            0xbc9b
#define SVM_CODE_16_R0_ATTR            0xb09b
#define SVM_CODE_16_R0_CO_ATTR         0xb09f
#define SVM_CODE_16_R3_ATTR            0xb0fb

#define SVM_DATA_32_R0_ATTR            0xbc93
#define SVM_DATA_16_R0_ATTR            0xb093
#define SVM_DATA_16_R3_ATTR            0xb0f3

#define __set_code16r0_desc(_attr_)    (_attr_.raw = SVM_CODE_16_R0_ATTR)
#define __set_data16r0_desc(_attr_)    (_attr_.raw = SVM_DATA_16_R0_ATTR)
#define __set_data16r3_desc(_attr_)    (_attr_.raw = SVM_DATA_16_R3_ATTR)

/*
** Fields synchro un-needed under SVM
*/
#define __pre_access(_fld_)                     ({})
#define __set_accessed(_fld_)                   ({})
#define __post_access(_fld_)                    ({})
#define __cond_access(_wr_,_fld_)               ({})

/*
** Paging & TLBs
*/
#include <svm_npt.h>

#define npg_init()                     svm_npt_map()
#define npg_cr3                        __ctrls.ncr3
#define npg_dft_attr                   ((uint64_t)(PG_USR|PG_RW))
#define npg_dft_attr_nx                ((uint64_t)(PG_USR|PG_RW|PG_NX))
#define npg_get_attr(_e)               pg_get_attr(_e)
#define npg_present(_e)                pg_present(_e)
#define npg_large(_e)                  pg_large(_e)
#define npg_zero(_e)                   pg_zero(_e)

#define npg_set_entry(_e,_a,_p)            pg_set_entry(_e,_a,_p)
#define npg_set_page_entry(_e,_a,_p)       pg_set_entry(_e,_a,_p)
#define npg_set_large_page_entry(_e,_a,_p) pg_set_large_entry(_e,_a,_p)

#define npg_pml4e_t                    pml4e_t
#define npg_pdpe_t                     pdpe_t
#define npg_pde64_t                    pde64_t
#define npg_pte64_t                    pte64_t

#define npg_pml4_t                     pml4_t
#define npg_pdp_t                      pdp_t
#define npg_pd64_t                     pd64_t
#define npg_pt64_t                     pt64_t

#define svm_reset_tlb_control()					\
   ({								\
      if(__ctrls.tlb_ctrl.tlb_control != VMCB_TLB_CTL_NONE)	\
	 __ctrls.tlb_ctrl.tlb_control = VMCB_TLB_CTL_NONE;	\
   })

#define npg_invlpg(_va)          invlpga((offset_t)_va)
#define __flush_asid_tlbs(_t)    (__ctrls.tlb_ctrl.tlb_control = _t)
#define __flush_tlb()            __flush_asid_tlbs(info->vm.cpu.skillz.flush_tlb)
#define __flush_tlb_glb()        __flush_asid_tlbs(info->vm.cpu.skillz.flush_tlb_glb)

#define __update_npg_cache(gcr3)	\
   ({					\
      npg_cr3.pml4.pwt = (gcr3)->pwt;	\
      npg_cr3.pml4.pcd = (gcr3)->pcd;	\
   })

#define npg_cr3_set(_addr)              (npg_cr3.addr = page_nr((_addr)))

/* XXX: check bits violation (cf. 15.25.10) */
#define __update_npg_pdpe()		({1;})
#define npg_get_asid()	                (__ctrls.tlb_ctrl.guest_asid)
#define npg_set_asid(_x)                ({__ctrls.tlb_ctrl.guest_asid = (_x);})

#define npg_err_t                       vmcb_exit_info_npf_t
#define npg_error_not_present(_e)       (_e.p == 0)
#define npg_error_execute(_e)           (_e.id)

#define npg_error()                     (__ctrls.exit_info_1.npf)
#define npg_fault()                     (__ctrls.exit_info_2.raw)

/*
** CR0 cache
*/
#define __cr0_cache_update(_gst)	\
   ({					\
      cr0_reg_t cr0;			\
      cr0.raw = get_cr0();		\
      cr0.cd = (_gst)->cd;		\
      cr0.nw = (_gst)->nw;		\
      set_cr0(cr0.raw);			\
   })

#define __cr0_update(_gst)	\
   ({				\
      __cr0.low = (_gst)->low;	\
   })

#define __cr4_update(_gst)	\
   ({				\
      __cr4.low = (_gst)->low;	\
   })

/*
** Masks
*/
#define __exception_bitmap      (__ctrls.exception_bitmap)
#define __cr_rd_bitmap          (__ctrls.cr_read_bitmap)
#define __cr_wr_bitmap          (__ctrls.cr_write_bitmap)

#define __update_exception_mask()	\
   ({					\
      __exception_bitmap.raw =		\
	 info->vm.cpu.dflt_excp  |	\
	 info->vmm.ctrl.usr.excp |	\
	 info->vmm.ctrl.dbg.excp;	\
   })

#define __update_cr_read_mask()				\
   ({ __cr_rd_bitmap = VMCB_CR_R_BITMAP | info->vmm.ctrl.usr.cr_rd; })

#define __update_cr_write_mask()			\
   ({ __cr_wr_bitmap = VMCB_CR_W_BITMAP | info->vmm.ctrl.usr.cr_wr; })

#define __allow_dr_access()		\
   ({					\
      __ctrls.dr_read_bitmap = 0;	\
      __ctrls.dr_write_bitmap = 0;	\
   })

#define __deny_dr_access()		\
   ({					\
      __ctrls.dr_read_bitmap = -1;	\
      __ctrls.dr_write_bitmap = -1;	\
   })

/*
** Instructions
*/
#define __vmexit_on_insn()						\
   (__svm_vmexit_on_insn() || info->vm.cpu.emu_sts == EMU_STS_DONE)

/* XXX
** we may have prefix etc before a standard instruction
** instead of directly giving size (ie. invd=2, int1=1...)
** we should refer to our disasm engine
*/
#define __insn_sz()				\
   ({						\
      debug_warning();				\
      0;
   })


#define __allow_pushf()	         ({ __ctrls.sys_insn_bitmap.pushf = 0; })
#define __deny_pushf()	         ({ __ctrls.sys_insn_bitmap.pushf = 1; })
#define __allow_popf()	         ({ __ctrls.sys_insn_bitmap.popf  = 0; })
#define __deny_popf()            ({ __ctrls.sys_insn_bitmap.popf  = 1; })
#define __allow_iret()           ({ __ctrls.sys_insn_bitmap.iret  = 0; })
#define __deny_iret()            ({ __ctrls.sys_insn_bitmap.iret  = 1; })
#define __allow_soft_int()       ({ __ctrls.sys_insn_bitmap.intn  = 0; })
#define __deny_soft_int()        ({ __ctrls.sys_insn_bitmap.intn  = 1; })
#define __allow_hrdw_int()       ({ __ctrls.sys_insn_bitmap.intr  = 0; })
#define __deny_hrdw_int()        ({ __ctrls.sys_insn_bitmap.intr  = 1; })
#define __allow_icebp()          ({ __ctrls.vmm_insn_bitmap.icebp = 0; })
#define __deny_icebp()           ({ __ctrls.vmm_insn_bitmap.icebp = 1; })

#define __allow_sgdt()           ({ __ctrls.sys_insn_bitmap.gdtr_read = 0; })
#define __deny_sgdt()            ({ __ctrls.sys_insn_bitmap.gdtr_read = 1; })
#define __allow_sidt()           ({ __ctrls.sys_insn_bitmap.idtr_read = 0; })
#define __deny_sidt()            ({ __ctrls.sys_insn_bitmap.idtr_read = 1; })
#define __allow_sldt()           ({ __ctrls.sys_insn_bitmap.ldtr_read = 0; })
#define __deny_sldt()            ({ __ctrls.sys_insn_bitmap.ldtr_read = 1; })

#define __allow_lgdt()           ({ __ctrls.sys_insn_bitmap.gdtr_write = 0; })
#define __deny_lgdt()            ({ __ctrls.sys_insn_bitmap.gdtr_write = 1; })
#define __allow_lidt()           ({ __ctrls.sys_insn_bitmap.idtr_write = 0; })
#define __deny_lidt()            ({ __ctrls.sys_insn_bitmap.idtr_write = 1; })
#define __allow_lldt()           ({ __ctrls.sys_insn_bitmap.ldtr_write = 0; })
#define __deny_lldt()            ({ __ctrls.sys_insn_bitmap.ldtr_write = 1; })

#define __allow_gdt_access()     ({ __allow_sgdt(); __allow_lgdt(); })
#define __deny_gdt_access()      ({ __deny_sgdt(); __deny_lgdt(); })

/*
** Events
*/
#define __exit_reason            __ctrls.exit_code.low
#define __vmexit_on_excp()       __svm_vmexit_on_excp()
#define __exception_vector      (__ctrls.exit_code.low - SVM_VMEXIT_EXCP_START)
#define __exception_error        __ctrls.exit_info_1.excp
#define __exception_fault       (__ctrls.exit_info_2.raw & 0xffffffffULL)
#define __injecting_exception() (__ctrls.event_injection.v?1:0)

#define __svm_prepare_event_injection(_ev, _type, _vector)		\
   ({									\
      _ev.raw    = 0ULL;						\
      _ev.vector = _vector;						\
      _ev.type   = _type;						\
      _ev.v      = 1;							\
   })

#define __setup_iwe(_on_,_nr_)					\
   __svm_vmexit_setup_interrupt_window_exiting(_on_,_nr_)

#define __inject_intn(n)            __svm_vmexit_inject_intn(n)
#define __inject_exception(a,b,c)   __svm_vmexit_inject_exception(a,b,c)
#define __inject_intr(a)            __svm_vmexit_inject_interrupt(a)

#define __interrupt_shadow         (__ctrls.int_shadow)
#define __interrupts_on()          (__rflags.IF)
#define __safe_interrupts_on()     (__interrupts_on() && !__interrupt_shadow.low)
#define __iwe_on()                 (__ctrls.int_ctrl.v_irq)

/*
** MSRs and IOs
*/
#define __resolve_msr_arch(_wr_)          __svm_vmexit_resolve_msr(_wr_)
#define __resolve_cpuid_arch(_idx_)       __svm_vmexit_resolve_cpuid(_idx_)

#define __allow_io(_p_)                   svm_allow_io(info->vm.cpu.vmc,_p_)
#define __deny_io(_p_)                    svm_deny_io(info->vm.cpu.vmc,_p_)
#define __allow_io_range(_p1,_p2)         svm_allow_io_range(info->vm.cpu.vmc,_p1,_p2)
#define __deny_io_range(_p1,_p2)          svm_deny_io_range(info->vm.cpu.vmc,_p1,_p2)

#define __string_io_linear(_tgt,_iO)					\
   ({									\
      _tgt = (&__state.es + vm_ctrls.exit_info_1.io.seg)->base.raw;	\
      if(_iO->in)							\
	 _tgt += (info->vm.cpu.gpr->rdi.raw & _iO->msk);		\
      else								\
	 _tgt += (info->vm.cpu.gpr->rsi.raw & _iO->msk);		\
   })

#define __efer_update(pg)		  ({})

/*
** Last Branch Record
*/
#define __enable_lbr()            ({ __state.dbgctl.lbr = 1; })
#define __disable_lbr()           ({ __state.dbgctl.lbr = 0; })
#define __setup_lbr()             ({})

#define __lbr_from()              (__cs.base.raw+__state.lbr_from.raw)
#define __lbr_to()                (__cs.base.raw+__state.lbr_to.raw)

/* XXX: what if IDT handler is not related to CS segment */
#define __lbr_from_excp()         (__cs.base.raw+__state.lbr_from_excp.raw)
#define __lbr_to_excp()	          (__cs.base.raw+__state.lbr_to_excp.raw)

/*
** Functions
*/
#ifdef __INIT__
void  svm_vm_init();
#endif

#endif
