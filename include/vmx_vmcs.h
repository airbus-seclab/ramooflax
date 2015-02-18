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
#ifndef __VMX_VMCS_H__
#define __VMX_VMCS_H__

#include <types.h>
#include <segmem.h>
#include <pagemem.h>
#include <excp.h>
#include <asm.h>
#include <print.h>

#include <vmx_types.h>
#include <vmx_vmcs_acc.h>
#include <vmx_msr.h>
#include <vmx_ept.h>

#ifndef __INIT__
#include <vmx_exit.h>
#endif

/*
** VMCS guest state area
*/
typedef struct vmcs_guest_state_area_segment_descriptor
{
   vmcs_t(seg_sel_t,             selector);
   vmcs_t(raw64_t,               base);
   vmcs_t(raw32_t,               limit);
   vmcs_t(vmcs_guest_seg_attr_t, attributes);

} __attribute__((packed)) vmcs_guest_seg_desc_t;

typedef struct vmcs_guest_state_area_descriptor_table_register
{
   vmcs_t(raw64_t,    base);
   vmcs_t(raw32_t,    limit);

} __attribute__((packed)) vmcs_guest_dt_reg_t;

typedef struct vmcs_guest_register_state_area
{
   vmcs_t(cr0_reg_t,              cr0);
   vmcs_t(raw64_t,                cr2); /* fake */
   vmcs_t(cr3_reg_t,              cr3);
   vmcs_t(cr4_reg_t,              cr4);
   vmcs_t(dr6_reg_t,              dr6); /* fake */
   vmcs_t(dr7_reg_t,              dr7);

   vmcs_t(raw64_t,                rsp);
   vmcs_t(raw64_t,                rip);
   vmcs_t(rflags_reg_t,           rflags);

   vmcs_guest_seg_desc_t          es, cs, ss, ds, fs, gs, ldtr, tr;
   vmcs_guest_dt_reg_t            gdtr, idtr;

   vmcs_t(ia32_dbg_ctl_msr_t,     ia32_dbgctl);
   vmcs_t(raw32_t,                ia32_sysenter_cs);
   vmcs_t(raw64_t,                ia32_sysenter_esp);
   vmcs_t(raw64_t,                ia32_sysenter_eip);

   vmcs_t(ia32_perf_glb_ctl_msr_t, ia32_perf);
   vmcs_t(pat_t,                   ia32_pat);
   vmcs_t(ia32_efer_msr_t,         ia32_efer);

   vmcs_t(raw32_t,                smbase);

} __attribute__((packed)) vmcs_guest_reg_state_t;

typedef struct vmcs_guest_non_register_state_area
{
   vmcs_t(raw32_t,                         activity_state);
   vmcs_t(vmcs_guest_int_state_t,          interrupt);
   vmcs_t(vmcs_guest_pending_dbg_excp_t,   dbg_excp);
   vmcs_t(raw64_t,                         vmcs_link_ptr);
   vmcs_t(raw32_t,                         preempt_timer);
   vmcs_t(pdpe_t,                          pdpe_0);
   vmcs_t(pdpe_t,                          pdpe_1);
   vmcs_t(pdpe_t,                          pdpe_2);
   vmcs_t(pdpe_t,                          pdpe_3);

} __attribute__((packed)) vmcs_guest_non_reg_state_t;

typedef struct vmcs_guest_state_area
{
   vmcs_guest_reg_state_t;
   vmcs_guest_non_reg_state_t;

} __attribute__((packed)) vmcs_guest_t;


/*
** VMCS host state area
*/
typedef struct vmcs_host_state_area
{
   vmcs_t(cr0_reg_t,     cr0);
   vmcs_t(cr3_reg_t,     cr3);
   vmcs_t(cr4_reg_t,     cr4);

   vmcs_t(raw64_t,       rsp);
   vmcs_t(raw64_t,       rip);

   vmcs_t(seg_sel_t,     cs);
   vmcs_t(seg_sel_t,     ss);
   vmcs_t(seg_sel_t,     ds);
   vmcs_t(seg_sel_t,     es);
   vmcs_t(seg_sel_t,     fs);
   vmcs_t(seg_sel_t,     gs);
   vmcs_t(seg_sel_t,     tr);

   vmcs_t(raw64_t,       fs_base);
   vmcs_t(raw64_t,       gs_base);
   vmcs_t(raw64_t,       tr_base);
   vmcs_t(raw64_t,       gdtr_base);
   vmcs_t(raw64_t,       idtr_base);

   vmcs_t(raw32_t,       ia32_sysenter_cs);
   vmcs_t(raw64_t,       ia32_sysenter_esp);
   vmcs_t(raw64_t,       ia32_sysenter_eip);

   vmcs_t(ia32_perf_glb_ctl_msr_t, ia32_perf);
   vmcs_t(pat_t,                   ia32_pat);
   vmcs_t(ia32_efer_msr_t,         ia32_efer);

} __attribute__((packed)) vmcs_host_t;

/*
** VMCS VM execution controls
*/
typedef struct vmcs_execution_controls
{
   vmcs_t(vmcs_exec_ctl_pin_based_t,    pin);
   vmcs_t(vmcs_exec_ctl_proc_based_t,   proc);
   vmcs_t(vmcs_exec_ctl_proc2_based_t,  proc2);

   vmcs_t(raw32_t,            excp_bitmap);
   vmcs_t(excp32_err_code_t,  pagefault_err_code_mask);
   vmcs_t(excp32_err_code_t,  pagefault_err_code_match);

   /*
   ** IO bitmaps physical addrs
   ** 4KB size and aligned
   **
   ** a: io in range 0x0000 - 0x7fff
   ** b: io in range 0x8000 - 0xffff
   */
   vmcs_t(raw64_t,     io_bitmap_a);
   vmcs_t(raw64_t,     io_bitmap_b);

   vmcs_t(raw64_t,     tsc_offset);    /* rdtsc signed offset */

   vmcs_t(cr0_reg_t,   cr0_mask);
   vmcs_t(cr4_reg_t,   cr4_mask);

   vmcs_t(cr0_reg_t,   cr0_read_shadow);
   vmcs_t(cr4_reg_t,   cr4_read_shadow);

   vmcs_t(cr3_reg_t,   cr3_target_0);
   vmcs_t(cr3_reg_t,   cr3_target_1);
   vmcs_t(cr3_reg_t,   cr3_target_2);
   vmcs_t(cr3_reg_t,   cr3_target_3);
   vmcs_t(raw32_t,     cr3_target_count);

   vmcs_t(raw64_t,     apic_addr);
   vmcs_t(raw64_t,     vapic_addr);
   vmcs_t(raw64_t,     tpr_threshold);

   /*
   ** MSR bitmaps physical addrs, each 1KB
   **
   ** low : 0x00000000 - 0x00001fff
   ** high: 0xc0000000 - 0xc0001fff
   **
   */
   vmcs_t(raw64_t,       msr_bitmap);

   vmcs_t(raw64_t,       executive_vmcs_ptr);

   vmcs_t(vmcs_eptp_t,   eptp);
   vmcs_t(raw16_t,       vpid);

   vmcs_t(raw32_t,       ple_gap);
   vmcs_t(raw32_t,       ple_win);

} __attribute__((packed)) vmcs_exec_ctl_t;

/*
** VMCS vm exit/entry control msr store/load area entry
*/
typedef struct vmcs_control_msr_area_entry
{
   uint32_t    msr_index;
   uint32_t    reserved;
   raw64_t     msr_data;

} __attribute__((packed)) vmcs_ctl_msr_area_entry_t;

/*
** VMCS VM exit controls
*/
typedef struct vmcs_exit_controls
{
   vmcs_t(vmcs_exit_ctl_vect_t,  exit);
   vmcs_t(raw32_t,               msr_store_count);
   vmcs_t(raw64_t,               msr_store_addr);
   vmcs_t(raw32_t,               msr_load_count);
   vmcs_t(raw64_t,               msr_load_addr);

} __attribute__((packed)) vmcs_exit_ctl_t;

/*
** VMCS VM entry controls
*/
typedef struct vmcs_entry_controls
{
   vmcs_t(vmcs_entry_ctl_vect_t,     entry);
   vmcs_t(raw32_t,                   msr_load_count);
   vmcs_t(raw64_t,                   msr_load_addr);

   vmcs_t(vmcs_entry_ctl_int_info_t, int_info); /* event injected */
   vmcs_t(raw32_t,                   err_code); /* err code injected */
   vmcs_t(raw32_t,                   insn_len);

} __attribute__((packed)) vmcs_entry_ctl_t;

/*
** VMCS VM controls
*/
typedef struct vmcs_controls
{
   vmcs_exec_ctl_t    exec_ctrls;
   vmcs_exit_ctl_t    exit_ctrls;
   vmcs_entry_ctl_t   entry_ctrls;

} __attribute__((packed)) vmcs_ctl_t;

/*
** VMCS VM exit information fields
**
**         "Read Only"
*/
typedef struct vmcs_vm_exit_information
{
   vmcs_t(vmcs_exit_info_exit_reason_t,   reason);
   vmcs_t(vmcs_exit_info_qualif_t,        qualification);

   vmcs_t(raw64_t,                        guest_linear);
   vmcs_t(raw64_t,                        guest_physical);

   /* due to vectored events */
   vmcs_t(vmcs_exit_info_int_info_t,      int_info);
   vmcs_t(excp32_err_code_t,              int_err_code);

   /* during event delivery */
   vmcs_t(vmcs_exit_info_idt_vect_t,      idt_info);
   vmcs_t(excp32_err_code_t,              idt_err_code);

   /* vmexit on insn execution */
   vmcs_t(raw32_t,                        insn_len);
   vmcs_t(vmcs_exit_info_insn_t,          insn_info);

   /* due to smis after retirement of IO insn */
   vmcs_t(raw64_t,                        io_rcx);
   vmcs_t(raw64_t,                        io_rsi);
   vmcs_t(raw64_t,                        io_rdi);
   vmcs_t(raw64_t,                        io_rip);

   /* vmx instruction error */
   vmcs_t(vmx_insn_err_t,                 vmx_insn_err);

} __attribute__((packed)) vmcs_exit_info_t;

/*
**  VMCS region
*/
typedef struct vmcs_region
{
   uint32_t            revision_id;
   uint32_t            abort;

   vmcs_guest_t        guest_state;
   vmcs_host_t         host_state;
   vmcs_ctl_t          controls;
   vmcs_exit_info_t    exit_info;

} __attribute__((packed)) vmcs_region_t;

/*
** Maximum vmcs size (cf. IA32_VMX_BASIC msr)
*/
#define VMCS_CPU_REGION_SZ  0x1000

typedef union vmcs_cpu_region
{
   vmcs_region_t;
   uint8_t raw[VMCS_CPU_REGION_SZ];

} __attribute__((packed)) vmcs_cpu_region_t;

/*
** Exception bitmap macros
*/
#define __vmx_allow_exception(_vmcs_,_excp_)  ((_vmcs_).controls.exec_ctrls.excp_bitmap.raw &= ~(1<<(_excp_)))
#define __vmx_deny_exception(_vmcs_,_excp_)   ((_vmcs_).controls.exec_ctrls.excp_bitmap.raw |= (1<<(_excp_)))

/*
** I/O bitmap macros
*/
#define __vmx_allow_bitmap(_bm_,_x_)   ((_bm_)[(_x_)/8] &= ~(1<<((_x_)%8)))
#define __vmx_deny_bitmap(_bm_,_x_)    ((_bm_)[(_x_)/8] |=  (1<<((_x_)%8)))

#define __vmx_access_io(_x_,_access_,_vmc_)				\
   ({									\
      uint32_t io = (_x_);						\
      if( io < 0x8000 )							\
	 __vmx_##_access_##_bitmap((_vmc_)->io_bitmap_a,io);		\
      else								\
	 __vmx_##_access_##_bitmap((_vmc_)->io_bitmap_b,(io-0x8000));	\
   })

#define vmx_allow_io(_vmc_,_p_)        __vmx_access_io(_p_, allow, _vmc_)
#define vmx_deny_io(_vmc_,_p_)         __vmx_access_io(_p_, deny, _vmc_)

#define __vmx_access_io_range(_access_,_vmc_,_sp_,_ep_)		\
   ({								\
      uint32_t p;						\
      for( p=_sp_ ; p<=_ep_ ; p++ )				\
	 vmx_##_access_##_##io(_vmc_,p);			\
   })

#define vmx_allow_io_range(_vmc_,_sx_,_ex_)   __vmx_access_io_range(allow,_vmc_,_sx_,_ex_)
#define vmx_deny_io_range(_vmc_,_sx_,_ex_)    __vmx_access_io_range(deny,_vmc_,_sx_,_ex_)

/*
** MSR bitmap macros
*/
#define __vmx_access_msr(_x_,_r_,_w_,_access_,_bm_)			\
   ({									\
      uint32_t msr = (_x_);						\
      if( msr < 0x2000 )						\
      {									\
	 if( _r_ )							\
	    __vmx_##_access_##_bitmap((_bm_),msr);			\
	 if( _w_ )							\
	    __vmx_##_access_##_bitmap((_bm_)+2048,msr);			\
      }									\
      else if( range(msr,0xc0000000,0xc0002000-1) )			\
      {									\
	 if( _r_ )							\
	    __vmx_##_access_##_bitmap((_bm_)+1024,msr-0xc0000000);	\
	 if( _w_ )							\
	    __vmx_##_access_##_bitmap((_bm_)+3072,msr-0xc0000000);	\
      }									\
      else								\
	 panic( "can't access msr 0x%x !", msr );			\
   })

#define vmx_allow_msr_rd(_vmc_,_i_)     __vmx_access_msr(_i_, 1,0, allow, (_vmc_)->msr_bitmap)
#define vmx_deny_msr_rd(_vmc_,_i_)      __vmx_access_msr(_i_, 1,0, deny,  (_vmc_)->msr_bitmap)
#define vmx_allow_msr_wr(_vmc_,_i_)     __vmx_access_msr(_i_, 0,1, allow, (_vmc_)->msr_bitmap)
#define vmx_deny_msr_wr(_vmc_,_i_)      __vmx_access_msr(_i_, 0,1, deny,  (_vmc_)->msr_bitmap)
#define vmx_allow_msr_rw(_vmc_,_i_)     __vmx_access_msr(_i_, 1,1, allow, (_vmc_)->msr_bitmap)
#define vmx_deny_msr_rw(_vmc_,_i_)      __vmx_access_msr(_i_, 1,1, deny,  (_vmc_)->msr_bitmap)

#define __vmx_access_msr_range(_access_,_op_,_vmc_,_sp_,_ep_)		\
   ({									\
      uint32_t p;							\
      for( p=_sp_ ; p<=_ep_ ; p++ )					\
	 vmx_##_access_##_##msr##_##_op_(_vmc_,p);			\
   })

#define vmx_allow_msr_rd_range(_vmc_,_sx_,_ex_)  __vmx_access_msr_range(allow, rd, _vmc_,_sx_,_ex_)
#define vmx_deny_msr_rd_range(_vmc_,_sx_,_ex_)   __vmx_access_msr_range(deny,  rd, _vmc_,_sx_,_ex_)
#define vmx_allow_msr_wr_range(_vmc_,_sx_,_ex_)  __vmx_access_msr_range(allow, wr, _vmc_,_sx_,_ex_)
#define vmx_deny_msr_wr_range(_vmc_,_sx_,_ex_)   __vmx_access_msr_range(deny,  wr, _vmc_,_sx_,_ex_)
#define vmx_allow_msr_rw_range(_vmc_,_sx_,_ex_)  __vmx_access_msr_range(allow, rw, _vmc_,_sx_,_ex_)
#define vmx_deny_msr_rw_range(_vmc_,_sx_,_ex_)   __vmx_access_msr_range(deny,  rw, _vmc_,_sx_,_ex_)

/*
** Functions
*/
#ifdef __INIT__
void  vmx_vmcs_init();
#endif

#endif

