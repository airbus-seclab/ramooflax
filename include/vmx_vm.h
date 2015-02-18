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

/*
** System registers
*/
#define __state                 info->vm.vmcs.guest_state
#define __host_state            info->vm.vmcs.host_state
#define __exec_ctrls            info->vm.vmcs.controls.exec_ctrls
#define __exit_ctrls            info->vm.vmcs.controls.exit_ctrls
#define __entry_ctrls           info->vm.vmcs.controls.entry_ctrls
#define __exit_info             info->vm.vmcs.exit_info

#define __cr0_real              __state.cr0
#define __cr0                   __exec_ctrls.cr0_read_shadow
#define __cr2                   __state.cr2
#define __cr3                   __state.cr3
#define __cr4_real              __state.cr4
#define __cr4                   __exec_ctrls.cr4_read_shadow
#define __rip                   __state.rip
#define __rflags                __state.rflags

#define __dr6                   __state.dr6
#define __dr7                   __state.dr7

#define __cs                    __state.cs
#define __ds                    __state.ds
#define __es                    __state.es
#define __fs                    __state.fs
#define __gs                    __state.gs
#define __ss                    __state.ss

#define __cpl                   __state.cs.selector.rpl
#define __gdtr                  __state.gdtr
#define __idtr                  __state.idtr
#define __ldtr                  __state.ldtr
#define __tr                    __state.tr

#define __efer                  info->vm.efer
#define __dbgctl                __state.ia32_dbgctl
#define __sysenter_cs           __state.ia32_sysenter_cs
#define __sysenter_eip          __state.ia32_sysenter_eip
#define __sysenter_esp          __state.ia32_sysenter_esp

#include <cpu_modes.h>

/*
** Segmentation
*/
#define VMX_CODE_32_R0_ATTR                 0xc09b
#define VMX_CODE_16_R0_ATTR                 0x809b
#define VMX_CODE_16_R0_CO_ATTR              0x809f
#define VMX_CODE_16_R1_CO_ATTR              0x80bf
#define VMX_CODE_16_R3_ATTR                 0x80fb

#define VMX_DATA_32_R0_ATTR                 0xc093
#define VMX_DATA_16_R0_ATTR                 0x8093
#define VMX_DATA_16_R1_ATTR                 0x80b3
#define VMX_DATA_16_R3_ATTR                 0x80f3

#define __set_code16r0co_desc(_attr_)       (_attr_.raw = VMX_CODE_16_R0_CO_ATTR)
#define __set_data16r0_desc(_attr_)         (_attr_.raw = VMX_DATA_16_R0_ATTR)
#define __set_data16r3_desc(_attr_)         (_attr_.raw = VMX_DATA_16_R3_ATTR)

#define VMX_TSS_32_ATTR                     0x8b

#define __set_tss32_desc(_attr_)            (_attr_.raw = VMX_TSS_32_ATTR)

/*
** Fields synchro
*/
#define __pre_access(_fld_)                 vmcs_read(_fld_)
#define __set_accessed(_fld_)               vmcs_set_read(_fld_)
#define __post_access(_fld_)		    vmcs_dirty(_fld_)
#define __cond_access(_wr_,_fld_)           vmcs_cond(_wr_,_fld_)

/*
** Paging & TLBs
*/
#include <vmx_ept.h>

#define npg_init()                     vmx_ept_map()
#define npg_dft_attr                   ept_dft_attr
#define npg_dft_attr_nx                ept_dft_attr_nx
#define npg_get_attr(_e)               ept_pg_get_attr(_e)
#define npg_present(_e)                ept_pg_present(_e)
#define npg_large(_e)                  pg_large(_e)
#define npg_zero(_e)                   ept_zero(_e)

#define npg_set_entry(_e,_a,_p)            ept_pg_set_entry(_e,_a,_p)
#define npg_set_page_entry(_e,_a,_p)       ept_pg_set_page_entry(_e,_a,_p)
#define npg_set_large_page_entry(_e,_a,_p) ept_pg_set_large_page_entry(_e,_a,_p)

#define npg_pml4e_t                    ept_pml4e_t
#define npg_pdpe_t                     ept_pdpe_t
#define npg_pde64_t                    ept_pde64_t
#define npg_pte64_t                    ept_pte64_t

#define npg_pml4_t                     ept_pml4_t
#define npg_pdp_t                      ept_pdp_t
#define npg_pd64_t                     ept_pd64_t
#define npg_pt64_t                     ept_pt64_t

#define npg_invlpg(_va)          invept(VMCS_EPT_INV_ALL)
#define __flush_asid_tlbs(_t)    invvpid(_t)
#define __flush_tlb()            __flush_asid_tlbs(info->vm.cpu.skillz.flush_tlb)
#define __flush_tlb_glb()        __flush_asid_tlbs(info->vm.cpu.skillz.flush_tlb_glb)
#define __update_npg_cache(_x)	 ({})
#define __update_npg_pdpe()	 vmx_ept_update_pdpe()

#define npg_cr3                        __exec_ctrls.eptp
#define npg_cr3_set(_addr)				\
   ({							\
      __exec_ctrls.eptp.addr = page_nr((_addr));	\
      vmcs_dirty(__exec_ctrls.eptp);			\
   })

#define npg_get_asid()				\
   ({						\
      vmcs_read(__exec_ctrls.vpid);		\
      __exec_ctrls.vpid.raw;			\
   })

#define npg_set_asid(_x)			\
   ({						\
      __exec_ctrls.vpid.raw = (_x);		\
      vmcs_dirty(__exec_ctrls.vpid);		\
   })

#define npg_err_t                               vmcs_exit_info_ept_t
#define npg_error_not_present(_e)               (((((_e.raw)>>3)&7) == 0)?1:0)
#define npg_error_execute(_e)                   (_e.x)

#define npg_error()				\
   ({						\
      vmcs_read(__exit_info.qualification);	\
      __exit_info.qualification.ept;		\
   })

#define npg_fault()				\
   ({						\
      vmcs_read(__exit_info.guest_physical);	\
      __exit_info.guest_physical.raw;		\
   })

/*
** Masks
*/
#define __exception_bitmap      __exec_ctrls.excp_bitmap
#define __update_exception_mask()		\
   ({						\
      __exception_bitmap.raw =			\
	 info->vm.cpu.dflt_excp  |		\
	 info->vmm.ctrl.usr.excp |		\
	 info->vmm.ctrl.dbg.excp;		\
      vmcs_dirty(__exec_ctrls.excp_bitmap);	\
   })

/*
** Instructions
*/
#define __vmexit_on_insn()						\
   (__vmx_vmexit_on_insn() || info->vm.cpu.emu_sts == EMU_STS_DONE)

#define __insn_sz()				\
   ({						\
      vmcs_read(__exit_info.insn_len);		\
      __exit_info.insn_len.raw;			\
   })

/*
** CR registers
*/
#define __cr0_cache_update(_guest)			\
   ({							\
      /* XXX: set EPT as UC */				\
      debug(CR0, "cr0 cache update not implemented\n");	\
   })

#define __cr0_update(_gst)			\
   ({						\
      __cr0.low = __cr0_real.low = (_gst)->low;	\
      __post_access(__cr0);			\
      __post_access(__cr0_real);		\
   })

#define __cr4_update(_gst)			\
   ({						\
      __cr4.low = __cr4_real.low = (_gst)->low;	\
      __post_access(__cr4);			\
      __post_access(__cr4_real);		\
   })


#ifndef __INIT__
#include <vmx_exit_cr.h>
#endif

#define __update_cr_read_mask()	 __vmx_vmexit_cr_update_mask()
#define __update_cr_write_mask() __vmx_vmexit_cr_update_mask()

#define __allow_dr_access()		\
   ({					\
      vmcs_read(vm_exec_ctrls.proc);	\
      vm_exec_ctrls.proc.mdr = 0;	\
      vmcs_dirty(vm_exec_ctrls.proc);	\
   })

#define __deny_dr_access()		\
   ({					\
      vmcs_read(vm_exec_ctrls.proc);	\
      vm_exec_ctrls.proc.mdr = 1;	\
      vmcs_dirty(vm_exec_ctrls.proc);	\
   })

#define __allow_pushf()	         (XXX)
#define __deny_pushf()	         (XXX)
#define __allow_popf()	         (XXX)
#define __deny_popf()            (XXX)
#define __allow_iret()           (XXX)
#define __deny_iret()            (XXX)

#define __allow_dt_access()		\
   ({					\
      vmcs_read(vm_exec_ctrls.proc2);	\
      vm_exec_ctrls.proc2.dt = 0;	\
      vmcs_dirty(vm_exec_ctrls.proc2);	\
   })

#define __deny_dt_access()		\
   ({					\
      vmcs_read(vm_exec_ctrls.proc2);	\
      vm_exec_ctrls.proc2.dt = 1;	\
      vmcs_dirty(vm_exec_ctrls.proc2);	\
   })

#define __allow_gdt_access()     ({ __allow_dt_access(); })
#define __deny_gdt_access()      ({ __deny_dt_access(); })

#define __allow_soft_int()						\
   ({									\
      __allow_dt_access();						\
      __idtr.limit.wlow = info->vm.idt_limit;				\
      vmcs_dirty(__idtr.limit);						\
   })

#define __deny_soft_int()						\
   ({									\
      __deny_dt_access();						\
      vmcs_read(__idtr.limit);						\
      info->vm.idt_limit = __idtr.limit.wlow;				\
      __idtr.limit.wlow = BIOS_MISC_INTERRUPT*sizeof(ivt_e_t) - 1;	\
      vmcs_dirty(__idtr.limit);						\
   })

#define __allow_hrdw_int()       (XXX)
#define __deny_hrdw_int()        (XXX)
#define __allow_icebp()          (XXX)
#define __deny_icebp()           (XXX)


/*
** Events
*/
#define __exit_reason              __exit_info.reason.basic
#define __vmexit_on_excp()         __vmx_vmexit_on_excp()
#define __exception_vector	   __exit_info.int_info.vector
#define __exception_error          __exit_info.int_err_code
#define __exception_fault         (__exit_info.qualification.raw & 0xffffffffUL)

#define __clear_event_injection()  __vmx_clear_event_injection()
#define __injecting_event()       (__entry_ctrls.int_info.v?1:0)
#define __injected_event_type      __entry_ctrls.int_info.type
#define __injected_event()			\
   ({						\
      raw64_t _ev;				\
      _ev.low  = __entry_ctrls.int_info.raw;	\
      _ev.high = __entry_ctrls.err_code.raw;	\
      _ev.raw;					\
   })

#define __vmx_prepare_event_injection(_ev, _type, _vector)	\
   ({								\
      _ev.raw    = 0;						\
      _ev.vector = _vector;					\
      _ev.type   = _type;					\
      _ev.v      = 1;						\
      vmcs_dirty(_ev);						\
   })

#define __setup_iwe(_on_,_nr_)   \
   __vmx_vmexit_setup_interrupt_window_exiting(_on_,_nr_)

#define __inject_intn(n)            __vmx_vmexit_inject_intn(n)
#define __inject_exception(a,b,c)   __vmx_vmexit_inject_exception(a,b,c)
#define __inject_intr(a)            __vmx_vmexit_inject_interrupt(a)

#define __interrupt_shadow					\
   ({								\
      vmcs_read(__state.interrupt);				\
      (__state.interrupt.sti || __state.interrupt.mss);		\
   })
#define __interrupts_on()           __rflags.IF
#define __safe_interrupts_on()     (__interrupts_on() && !__interrupt_shadow)
#define __iwe_on()                  __exec_ctrls.proc.iwe

/*
** MSRs and IOs
*/
#define __resolve_msr_arch(_wr_)          __vmx_vmexit_resolve_msr(_wr_)
#define __resolve_cpuid_arch(_idx_)       __vmx_vmexit_resolve_cpuid(_idx_)

#define __allow_io(_p_)                   vmx_allow_io(info->vm.cpu.vmc,_p_)
#define __deny_io(_p_)                    vmx_deny_io(info->vm.cpu.vmc,_p_)
#define __allow_io_range(_p1,_p2)         vmx_allow_io_range(info->vm.cpu.vmc,_p1,_p2)
#define __deny_io_range(_p1,_p2)          vmx_deny_io_range(info->vm.cpu.vmc,_p1,_p2)

#define __string_io_linear(_tgt,_iO)			\
   (_tgt = (__exit_info.guest_linear.raw & (_iO->msk)))

#define __vmx_update_efer_lma()				\
   ({							\
      vmcs_read(vm_state.ia32_efer);			\
      vmcs_read(vm_entry_ctrls.entry);			\
      vm_entry_ctrls.entry.ia32e =			\
	 vm_state.ia32_efer.lme  =			\
	 vm_state.ia32_efer.lma  = info->vm.efer.lma;	\
      vmcs_dirty(vm_entry_ctrls.entry);			\
      vmcs_dirty(vm_state.ia32_efer);			\
   })

#define __efer_update(pg)			\
   ({						\
      int __up = 0;				\
						\
      if(pg && __cr4.pae && info->vm.efer.lme)	\
      {						\
	 info->vm.efer.lma = 1;			\
	 __up = 1;				\
      }						\
      else if(!pg && info->vm.efer.lma)		\
      {						\
	 info->vm.efer.lma = 0;			\
	 __up = 1;				\
      }						\
      						\
      if(__up)					\
	 __vmx_update_efer_lma();		\
   })

#define vmx_disable_preempt_timer()			\
   ({							\
      vmcs_read(vm_exit_ctrls.exit);			\
      vmcs_read(vm_exec_ctrls.pin);			\
      vm_exit_ctrls.exit.save_preempt_timer = 0;	\
      vm_exec_ctrls.pin.preempt = 0;			\
      vmcs_dirty(vm_exec_ctrls.pin);			\
      vmcs_dirty(vm_exit_ctrls.exit);			\
   })

/*
** Last Branch Record
*/
#define __enable_lbr()				\
   ({						\
      /* XXX: protect MSR access from guest */	\
      __state.ia32_dbgctl.lbr = 1;		\
      __state.ia32_dbgctl.freeze_smm_en = 1;	\
      vmcs_dirty(__state.ia32_dbgctl);		\
   })

#define __disable_lbr()				\
   ({						\
      /* XXX: release MSR access from guest */	\
      __state.ia32_dbgctl.lbr = 0;		\
      __state.ia32_dbgctl.freeze_smm_en = 0;	\
      vmcs_dirty(__state.ia32_dbgctl);		\
   })

#define __setup_lbr()				\
   ({						\
      msr_t m;					\
      rd_msr64(LBR_TOS_MSR, m.edx, m.eax);	\
      info->vm.lbr_tos = m.raw;			\
   })

#define __lbr_from()       __lbr_nehalem(info->vm.lbr_tos+LBR_FROM_IP_MSR)
#define __lbr_to()         __lbr_nehalem(info->vm.lbr_tos+LBR_TO_IP_MSR)
#define __lbr_from_excp()  ({msr_t m;rd_msr64(LBR_LER_FROM_LIP_MSR,m.edx,m.eax);m.raw;})
#define __lbr_to_excp()    ({msr_t m;rd_msr64(LBR_LER_TO_LIP_MSR,m.edx,m.eax);m.raw;})

/*
** Functions
*/
#ifdef __INIT__
void  vmx_vm_init();
#endif

#endif
