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
#ifndef __SVM_VMCB_H__
#define __SVM_VMCB_H__

#include <types.h>
#include <segmem.h>
#include <excp.h>
#include <msr.h>
#include <asm.h>
#include <print.h>

#ifndef __INIT__
#include <svm_exit.h>
#endif

/*
** VMCB System Instruction Bitmap
*/
typedef union vmcb_ctrl_sys_insn_bitmap
{
   struct
   {
      uint32_t   intr:1;
      uint32_t   nmi:1;
      uint32_t   smi:1;
      uint32_t   init:1;
      uint32_t   vintr:1;
      uint32_t   cr0_sel_write:1;
      uint32_t   idtr_read:1;
      uint32_t   gdtr_read:1;
      uint32_t   ldtr_read:1;
      uint32_t   tr_read:1;
      uint32_t   idtr_write:1;
      uint32_t   gdtr_write:1;
      uint32_t   ldtr_write:1;
      uint32_t   tr_write:1;
      uint32_t   rdtsc:1;
      uint32_t   rdpmc:1;
      uint32_t   pushf:1;
      uint32_t   popf:1;
      uint32_t   cpuid:1;
      uint32_t   rsm:1;
      uint32_t   iret:1;
      uint32_t   intn:1;
      uint32_t   invd:1;
      uint32_t   pause:1;
      uint32_t   hlt:1;
      uint32_t   invlpg:1;
      uint32_t   invlpga:1;
      uint32_t   io_insn:1;     /* uses IOs bitmap */
      uint32_t   msr_insn:1;    /* uses MSRs bitmap */
      uint32_t   task_sw:1;
      uint32_t   freez:1;
      uint32_t   shutdown:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcb_ctrl_sys_insn_bitmap_t;

/*
** VMCB VMM Instruction Bitmap
*/
typedef union vmcb_ctrl_vmm_insn_bitmap
{
   struct
   {
      uint32_t   vmrun:1;
      uint32_t   vmmcall:1;
      uint32_t   vmload:1;
      uint32_t   vmsave:1;
      uint32_t   stgi:1;
      uint32_t   clgi:1;
      uint32_t   skinit:1;
      uint32_t   rdtscp:1;
      uint32_t   icebp:1;
      uint32_t   wbinvd:1;
      uint32_t   monitor:1;
      uint32_t   mwait:1;
      uint32_t   mwait_cond:1; /* if monitor hardware is armed */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcb_ctrl_vmm_insn_bitmap_t;


/*
** Guest TLB control
*/
#define VMCB_TLB_CTL_NONE                   0
#define VMCB_TLB_CTL_FLUSH_ALL              1
#define VMCB_TLB_CTL_FLUSH_GUEST_ALL        3
#define VMCB_TLB_CTL_FLUSH_GUEST            7

typedef union vmcb_ctrl_tlb
{
   struct
   {
      uint64_t   guest_asid:32;
      uint64_t   tlb_control:8; /*
				** (0) do nothing
				** (1) flush TLB on VMRUN
				** (3) flush this guest TLB
				** (7) flush this guest non-global TLB
				*/
   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcb_ctrl_tlb_t;

/*
** Guest Interrupts control
*/
typedef union vmcb_ctrl_interrupt
{
   struct
   {
      uint64_t  v_tpr:8;
      uint64_t  v_irq:1;           /* (1) virtual interrupt pending */
      uint64_t  rsvrd0:7;
      uint64_t  v_intr_prio:4;     /* virtual interrupt priority */
      uint64_t  v_ign_tpr:1;       /* (1) current virtual interrupt ignore v_tpr */
      uint64_t  rsrvd1:3;
      uint64_t  v_intr_masking:1;  /* (1) guest RFLAGS.IF controls virtual interrupts
				   **     host  RFLAGS.IF controls physical interrupts
				   **
				   ** (0) RFLAGS.IF controls all interrupts
				   */
      uint64_t  rsrvd2:7;
      uint64_t  v_intr_vector:8;   /* vector to use for interrupt */
      uint64_t  rsrvd3:24;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcb_ctrl_int_t;


/*
** VMCB idt delivery, valid for:
**
** VMCB Exit Interrupt Information
** VMCB Interrupt Injection
**
*/
#define VMCB_IDT_DELIVERY_TYPE_EXT     0    /* physical or virtual interrupt */
#define VMCB_IDT_DELIVERY_TYPE_NMI     2
#define VMCB_IDT_DELIVERY_TYPE_EXCP    3
#define VMCB_IDT_DELIVERY_TYPE_SOFT    4

typedef union vmcb_idt_delivery
{
   struct
   {
      uint64_t  vector:8;     /* vector number */
      uint64_t  type:3;       /* exception/interrupt qualifier */
      uint64_t  ev:1;         /* exception push error code */
      uint64_t  rsrvd:19;     /* reserved */
      uint64_t  v:1;          /*
			      ** Exit Interrupt Info :
			      ** (1) we intercepted something while the guest
			      **  was delivering the following IDT info
			      **
			      ** Interrupt Injection :
			      ** (1) inject the following IDT info into guest
			      **
			      */
      uint64_t  err_code:32;  /* exception error code */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcb_idt_delivery_t;

typedef union vmcb_exit_info_io
{
   struct
   {
      uint64_t    d:1;      /* (1) IN (0) OUT */
      uint64_t    zero:1;   /* 0 */
      uint64_t    s:1;      /* string */
      uint64_t    rep:1;    /* repeat prefix */
      uint64_t    sz8:1;    /* operand size : 8,16,32 */
      uint64_t    sz16:1;
      uint64_t    sz32:1;
      uint64_t    addr16:1; /* address size : 16,32,64 */
      uint64_t    addr32:1;
      uint64_t    addr64:1;
      uint64_t    seg:3;    /* outs seg es=0,cs,ss,ds,fs,gs */
      uint64_t    rsrvd:3;
      uint64_t    port:16;  /* port number */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcb_exit_info_io_t;

typedef vmcb_exit_info_io_t svm_io_t;

typedef union vmcb_exit_info_smi
{
   struct
   {
      uint64_t    smi_rc:1; /* (1) External (0) Internal */
      uint64_t    rsrvd0:31;
      uint64_t    d:1;      /* (1) IN (0) OUT */
      uint64_t    val:1;    /* (1) I/O operation during SMI */
      uint64_t    s:1;      /* string */
      uint64_t    rep:1;    /* repeat prefix */
      uint64_t    sz8:1;    /* operand size : 8,16,32 */
      uint64_t    sz16:1;
      uint64_t    sz32:1;
      uint64_t    addr16:1; /* address size : 16,32,64 */
      uint64_t    addr32:1;
      uint64_t    addr64:1;
      uint64_t    rsrvd1:1;
      uint64_t    tf:1;     /* rflags.TF */
      uint64_t    brk:4;    /* i/o breakpoint */
      uint64_t    port:16;  /* port number */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcb_exit_info_smi_t;

typedef union vmcb_exit_info_npf
{
   struct
   {
      uint64_t    p:1;      /* non-present (0) */
      uint64_t    wr:1;     /* read (0), write (1)  */
      uint64_t    us:1;     /* kernel (0), user (1) */
      uint64_t    rsv:1;    /* reserved bits violation */
      uint64_t    id:1;     /* 1 insn fetch */
      uint64_t    r:27;     /* reserved */
      uint64_t    final:1;  /* while resolving guest final addr */
      uint64_t    ptb:1;    /* while resolving guest page table */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcb_exit_info_npf_t;

/*
** VMCB Exit Information 1
*/
typedef union vmcb_exit_info_1
{
   vmcb_exit_info_io_t   io;
   vmcb_exit_info_smi_t  smi;
   vmcb_exit_info_npf_t  npf;
   excp32_err_code_t     excp;
   raw64_t;

} __attribute__((packed)) vmcb_exit_info_1_t;

/*
** Control part of VMCB
*/
typedef struct vmcb_instruction_fetch
{
   uint8_t   sz;          /* number of bytes fetched */
   uint8_t   bytes[15];   /* instruction bytes */

} __attribute__((packed)) vmcb_insn_fetch_t;

typedef struct vmcb_controls_area_fields
{
   uint16_t                    cr_read_bitmap;  /* intercept cr0-15 read operations */
   uint16_t                    cr_write_bitmap; /* intercept cr0-15 write operations */
   uint16_t                    dr_read_bitmap;  /* intercept dr0-15 read operations */
   uint16_t                    dr_write_bitmap; /* intercept dr0-15 write operations */
   raw32_t                     exception_bitmap;/* intercept exceptions */
   vmcb_ctrl_sys_insn_bitmap_t sys_insn_bitmap; /* intercept system related insn */
   vmcb_ctrl_vmm_insn_bitmap_t vmm_insn_bitmap; /* intercept vmm related insn */
   uint8_t                     rsrvd0[42];
   uint16_t                    pause_filter;    /* pause filter count */
   raw64_t                     io_bitmap_addr;  /* 4KB aligned paddr IOs bitmap */
   raw64_t                     msr_bitmap_addr; /* 4KB aligned paddr MSRs bitmap */
   raw64_t                     tsc_offset;
   vmcb_ctrl_tlb_t             tlb_ctrl;        /* guest TLB control */
   vmcb_ctrl_int_t             int_ctrl;        /* guest interrupts control */
   raw64_t                     int_shadow;      /* (1) guest is in interrupt shadow */
   raw64_t                     exit_code;
   vmcb_exit_info_1_t          exit_info_1;
   raw64_t                     exit_info_2;
   vmcb_idt_delivery_t         exit_int_info;
   raw64_t                     npt;             /* (1) enable nested paging */
   uint8_t                     rsrvd1[16];
   vmcb_idt_delivery_t         event_injection;
   cr3_reg_t                   ncr3;            /* CR3 used for nested paging */
   raw64_t                     lbr;             /* (1) LBR virtualization */
   raw32_t                     clean_bits;      /* VMCB clean bits */
   raw64_t                     nrip;            /* next sequential rip */
   vmcb_insn_fetch_t           insn;            /* npt/intercepted #PF insn fetch */

} __attribute__((packed)) vmcb_ctrls_area_fields_t;

/*
** Guest segment descriptor state
*/
typedef union vmcb_segment_attributes
{
   struct
   {
      uint16_t    type:4;    /* anarchy on system segments */
      uint16_t    s:1;       /* (0) system type (1) non-system type */
      uint16_t    dpl:2;     /* descriptor privilege level */
      uint16_t    p:1;       /* segment present flag */
      uint16_t    avl:1;     /* available for fun and profit */
      uint16_t    l:1;       /* long mode */
      uint16_t    d:1;       /* 16/32 bits operands */
      uint16_t    g:1;       /* granularity */

   } __attribute__((packed));

   uint16_t  raw;

} __attribute__((packed)) vmcb_seg_attr_t;

typedef struct vmcb_segment_descriptor_t
{
   seg_sel_t        selector;
   vmcb_seg_attr_t  attributes;
   raw32_t          limit;
   raw64_t          base;

} __attribute__((packed)) vmcb_segment_desc_t;

/*
** Guest State part of VMCB
*/
typedef struct vmcb_state_area_fields
{
   vmcb_segment_desc_t  es, cs, ss, ds, fs, gs; /* only low32 base addr used */
   vmcb_segment_desc_t  gdtr, ldtr, idtr, tr;   /*
						** For IDTR and GDTR:
						** only lower 16 bits of limit are used
						** attributes and selector are reserved
						*/
   uint8_t              rsvrd0[43];
   uint8_t              cpl;                    /*
						** - force to 0 for real mode
						** - force to 3 for virtual-mode
						*/
   uint32_t             rsvrd1;
   amd_efer_msr_t       efer;
   uint8_t              rsvrd2[112];

   cr4_reg_t            cr4;
   cr3_reg_t            cr3;
   cr0_reg_t            cr0;
   dr7_reg_t            dr7;
   dr6_reg_t            dr6;
   rflags_reg_t         rflags;
   raw64_t              rip;
   uint8_t              rsvrd3[88];
   raw64_t              rsp;
   uint8_t              rsvrd4[24];
   raw64_t              rax;
   msr_t                star;
   msr_t                lstar;
   msr_t                cstar;
   msr_t                sfmask;
   msr_t                kernel_gs_base;
   msr_t                sysenter_cs;
   msr_t                sysenter_esp;
   msr_t                sysenter_eip;
   raw64_t              cr2;
   uint8_t              rsvrd5[32];
   msr_t                g_pat;
   amd_dbgctl_msr_t     dbgctl;
   raw64_t              lbr_from;
   raw64_t              lbr_to;
   raw64_t              lbr_from_excp;
   raw64_t              lbr_to_excp;

} __attribute__((packed)) vmcb_state_area_fields_t;


/*
** The SVM Virtual Machine Control Block
*/
#define VMCB_AREA_SZ           0x1000
#define VMCB_CTRLS_AREA_SZ     0x400
#define VMCB_STATE_AREA_SZ     (VMCB_AREA_SZ - VMCB_CTRLS_AREA_SZ)

typedef union vmcb_controls_area
{
   uint8_t raw[VMCB_CTRLS_AREA_SZ];
   vmcb_ctrls_area_fields_t;

} __attribute__((packed)) vmcb_ctrls_area_t;

typedef union vmcb_state_area
{
   uint8_t raw[VMCB_STATE_AREA_SZ];
   vmcb_state_area_fields_t;

} __attribute__((packed)) vmcb_state_area_t;

typedef union vmcb_area
{
   struct
   {
      vmcb_ctrls_area_t ctrls_area;
      vmcb_state_area_t state_area;

   } __attribute__((packed));

   uint8_t raw[VMCB_AREA_SZ];

} __attribute__((packed)) vmcb_area_t;

/*
** Exception bitmap macros
*/
#define __svm_allow_exception(_vmcb_,_excp_)  \
   ((_vmcb_).ctrls_area.exception_bitmap.raw &= ~(1<<(_excp_)))
#define __svm_deny_exception(_vmcb_,_excp_)   \
   ((_vmcb_).ctrls_area.exception_bitmap.raw |= (1<<(_excp_)))


/*
** IO bitmap macros
*/
#define __svm_allow_bitmap(_bm_,_x_)   (_bm_[(_x_)/8] &= ~(1<<((_x_)%8)))
#define __svm_deny_bitmap(_bm_,_x_)    (_bm_[(_x_)/8] |=  (1<<((_x_)%8)))

#define __svm_access_io_range(_access_,_vmc_,_sp_,_ep_)		\
   ({								\
      uint32_t p;						\
      for( p=_sp_ ; p<=_ep_ ; p++ )				\
	 svm_##_access_##_##io(_vmc_,p);			\
   })

#define svm_allow_io(_vmc_,_p_)			\
   __svm_allow_bitmap((_vmc_)->io_bitmap,_p_)
#define svm_deny_io(_vmc_,_p_)			\
   __svm_deny_bitmap((_vmc_)->io_bitmap,_p_)
#define svm_allow_io_range(_vmc_,_sx_,_ex_)	\
   __svm_access_io_range(allow,_vmc_,_sx_,_ex_)
#define svm_deny_io_range(_vmc_,_sx_,_ex_)	\
   __svm_access_io_range(deny,_vmc_,_sx_,_ex_)

/*
** MSR bitmap macros
*/
#define __svm_allow_bitmap_dual_rd(_bm_,_base_,_x_)	\
   (_bm_[(_base_)+(_x_)/4] &= ~(1<<(((_x_)%4)*2)))
#define __svm_deny_bitmap_dual_rd(_bm_,_base_,_x_)	\
   (_bm_[(_base_)+(_x_)/4] |=  (1<<(((_x_)%4)*2)))
#define __svm_allow_bitmap_dual_wr(_bm_,_base_,_x_)	\
   (_bm_[(_base_)+(_x_)/4] &= ~(1<<((((_x_)%4)*2)+1)))
#define __svm_deny_bitmap_dual_wr(_bm_,_base_,_x_)	\
   (_bm_[(_base_)+(_x_)/4] |=  (1<<((((_x_)%4)*2)+1)))
#define __svm_allow_bitmap_dual_rw(_bm_,_base_,_x_)	\
   (_bm_[(_base_)+(_x_)/4] &= ~(3<<(((_x_)%4)*2)))
#define __svm_deny_bitmap_dual_rw(_bm_,_base_,_x_)	\
   (_bm_[(_base_)+(_x_)/4] |=  (3<<(((_x_)%4)*2)))

#define __svm_access_msr(_x_,_access_,_op_,_bm_)			\
   ({									\
      uint32_t msr = (_x_);						\
      if( msr < 0x2000 )						\
	 __svm_##_access_##_bitmap_dual_##_op_(_bm_,0,msr);		\
      else if( range(msr,0xc0000000,0xc0001fff) )			\
	 __svm_##_access_##_bitmap_dual_##_op_(_bm_,0x800,(msr-0xbffff800)-0x800); \
      else if( range(msr,0xc0010000,0xc0011fff) )			\
	 __svm_##_access_##_bitmap_dual_##_op_(_bm_,0x1000,(msr-0xc000f000)-0x1000); \
      else								\
	 panic( "can't access msr 0x%x !", msr );			\
   })

#define svm_allow_msr_rd(_vmc_,_i_)				\
   __svm_access_msr(_i_, allow, rd, (_vmc_)->msr_bitmap)
#define svm_deny_msr_rd(_vmc_,_i_)				\
   __svm_access_msr(_i_, deny,  rd, (_vmc_)->msr_bitmap)
#define svm_allow_msr_wr(_vmc_,_i_)				\
   __svm_access_msr(_i_, allow, wr, (_vmc_)->msr_bitmap)
#define svm_deny_msr_wr(_vmc_,_i_)				\
   __svm_access_msr(_i_, deny,  wr, (_vmc_)->msr_bitmap)
#define svm_allow_msr_rw(_vmc_,_i_)				\
   __svm_access_msr(_i_, allow, rw, (_vmc_)->msr_bitmap)
#define svm_deny_msr_rw(_vmc_,_i_)				\
   __svm_access_msr(_i_, deny,  rw, (_vmc_)->msr_bitmap)

#define __svm_access_msr_range(_access_,_op_,_vmc_,_sp_,_ep_)		\
   ({									\
      uint32_t p;							\
      for( p=_sp_ ; p<=_ep_ ; p++ )					\
	 svm_##_access_##_##msr##_##_op_(_vmc_,p);			\
   })

#define svm_allow_msr_rd_range(_vmc_,_sx_,_ex_)		\
   __svm_access_msr_range(allow, rd, _vmc_,_sx_,_ex_)
#define svm_deny_msr_rd_range(_vmc_,_sx_,_ex_)		\
   __svm_access_msr_range(deny,  rd, _vmc_,_sx_,_ex_)
#define svm_allow_msr_wr_range(_vmc_,_sx_,_ex_)		\
   __svm_access_msr_range(allow, wr, _vmc_,_sx_,_ex_)
#define svm_deny_msr_wr_range(_vmc_,_sx_,_ex_)		\
   __svm_access_msr_range(deny,  wr, _vmc_,_sx_,_ex_)
#define svm_allow_msr_rw_range(_vmc_,_sx_,_ex_)		\
   __svm_access_msr_range(allow, rw, _vmc_,_sx_,_ex_)
#define svm_deny_msr_rw_range(_vmc_,_sx_,_ex_)		\
   __svm_access_msr_range(deny,  rw, _vmc_,_sx_,_ex_)

/*
** Functions
*/
#ifdef __INIT__
void   svm_vmcb_init();
#endif

#endif
