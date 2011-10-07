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

#include <vmx_vmcs_acc.h>
#include <vmx_msr.h>
#include <vmx_ept.h>

#ifndef __INIT__
#include <vmx_exit.h>
#endif

/*
** VMCS guest state area
*/
typedef union vmcs_guest_state_area_segment_attributes
{
   struct
   {
      uint32_t    type:4;     /* segment type */
      uint32_t    s:1;        /* descriptor type */
      uint32_t    dpl:2;      /* descriptor privilege level */
      uint32_t    p:1;        /* segment present flag */
      uint32_t    r1:4;       /* reserved */
      uint32_t    avl:1;      /* available for fun and profit */
      uint32_t    l:1;        /* long mode */
      uint32_t    d:1;        /* default length, depend on seg type */
      uint32_t    g:1;        /* granularity */
      uint32_t    u:1;        /* unusable segment (1) */
      uint32_t    r2:15;      /* reserved */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_guest_seg_attr_t;

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

#define VMX_VMCS_GUEST_ACTIVITY_STATE_ACTIVE    0
#define VMX_VMCS_GUEST_ACTIVITY_STATE_HALT      1
#define VMX_VMCS_GUEST_ACTIVITY_STATE_SHUTDOWN  2
#define VMX_VMCS_GUEST_ACTIVITY_STATE_SIPI      3

typedef union vmcs_guest_interruptibility_state
{
   struct
   {
      uint32_t   sti:1;  /* blocking by STI */
      uint32_t   mss:1;  /* blocking by mov ss */
      uint32_t   smi:1;  /* blocking by smi */
      uint32_t   nmi:1;  /* blocking by nmi */
      uint32_t   r:28;   /* reserved, must be set to 0 on VMentry */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_guest_int_state_t;

typedef union vmcs_guest_pending_debug_exceptions
{
   struct
   {
      uint64_t    b0:1;   /* DR7 correspondance */
      uint64_t    b1:1;
      uint64_t    b2:1;
      uint64_t    b3:1;
      uint64_t    r1:8;   /* reserved */
      uint64_t    be:1;   /* breakpoint enabled */
      uint64_t    r2:1;   /* reserverd */
      uint64_t    bs:1;   /* single step */
      uint64_t    r3:17;  /* reserved, must be set to 0 on VMentry */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcs_guest_pending_dbg_excp_t;

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
   vmcs_t(vmcs_guest_int_state_t,          int_state);
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
** VMCS VM execution controls pin based
** consult MSR VMX_PINBASED_CTLS
*/
typedef union vmcs_execution_pin_based_control
{
   struct
   {
      uint32_t   eint:1;      /* 0 ext intr cause vm exit (1) */
      uint32_t   r1:2;        /* 1-2 reserved */
      uint32_t   nmi:1;       /* 3 nmi cause vm exit (1) */
      uint32_t   r2:1;        /* 4 reserved */
      uint32_t   vnmi:1;      /* 5 virtual nmi control */
      uint32_t   preempt:1;   /* 6 enable vmx preemption timer */
      uint32_t   r3:25;       /* reserved */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_exec_ctl_pin_based_t;

/*
** VMCS VM execution controls processor based
** consult MSR VMX_PROCBASED_CTLS
*/
typedef union vmcs_execution_proc_based_control
{
   struct
   {
      uint32_t   r1:2;      /* 0-1   reserved */
      uint32_t   iwe:1;     /* 2     interrupt window exiting */
      uint32_t   tsc:1;     /* 3     rdtsc offset field modifier */
      uint32_t   r2:3;      /* 4-6   reserved */
      uint32_t   hlt:1;     /* 7     hlt exit */
      uint32_t   r3:1;      /* 8     reserved */
      uint32_t   invl:1;    /* 9     invlpg exit */
      uint32_t   mwait:1;   /* 10    mwait exit */
      uint32_t   rdpmc:1;   /* 11    rdpmc exit */
      uint32_t   rdtsc:1;   /* 12    rdtsc exit */
      uint32_t   r4:2;      /* 13-14 reserved */
      uint32_t   cr3l:1;    /* 15    wr cr3 exit */
      uint32_t   cr3s:1;    /* 16    rd cr3 exit */
      uint32_t   r5:2;      /* 17-18 reserved */
      uint32_t   cr8l:1;    /* 19    wr cr8 exit */
      uint32_t   cr8s:1;    /* 20    rd cr8 exit */
      uint32_t   tprs:1;    /* 21    TRP shadow */
      uint32_t   nwe:1;     /* 22    nmi window exiting */
      uint32_t   mdr:1;     /* 23    mov dr exit */
      uint32_t   ucio:1;    /* 24    unconditional IO exit */
      uint32_t   usio:1;    /* 25    use IO bitmaps */
      uint32_t   r6:1;      /* 26    reserved */
      uint32_t   mtf:1;     /* 27    monitor trap flag */
      uint32_t   umsr:1;    /* 28    use MSR bitmaps */
      uint32_t   mon:1;     /* 29    monitor exit */
      uint32_t   pause:1;   /* 30    pause exit */
      uint32_t   proc2:1;   /* 31    activate secondary controls */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_exec_ctl_proc_based_t;

typedef union vmcs_execution_secondary_proc_based_control
{
   struct
   {
      uint32_t   vapic:1;      /* 0     virtualize apic accesses */
      uint32_t   ept:1;        /* 1     enable EPT */
      uint32_t   dt:1;         /* 2     descriptor table exiting */
      uint32_t   rdtscp:1;     /* 3     rdtscp raises #UD */
      uint32_t   x2apic:1;     /* 4     virtualize x2apic mode */
      uint32_t   vpid:1;       /* 5     enable vpid */
      uint32_t   wbinvd:1;     /* 6     exit on wbinvd */
      uint32_t   uguest:1;     /* 7     unrestricted guest */
      uint32_t   r1:2;         /* 8-9   reserved */
      uint32_t   pause_loop:1; /* 10    pause loop exiting */
      uint32_t   r2:21;        /* 11-31 reserved */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_exec_ctl_proc2_based_t;

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
** VMCS vm exit control vector
** consult MSR VMX_EXIT_CTLS
*/
typedef union  vmcs_exit_control_vector
{
   struct
   {
      uint32_t      r1:2;
      uint32_t      save_dbgctl:1;
      uint32_t      r2:6;
      uint32_t      host_lmode:1;
      uint32_t      r3:2;
      uint32_t      load_ia32_perf:1;
      uint32_t      r4:2;
      uint32_t      ack_int:1;
      uint32_t      r5:2;
      uint32_t      save_ia32_pat:1;
      uint32_t      load_ia32_pat:1;
      uint32_t      save_ia32_efer:1;
      uint32_t      load_ia32_efer:1;
      uint32_t      save_preempt_timer:1;
      uint32_t      r6:9;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_exit_ctl_vect_t;

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
** VMCS vm entry control vector
** consult MSR VMX_ENTRY_CTLS
*/
typedef union vmcs_entry_control_vector
{
   struct
   {
      uint32_t      r1:2;
      uint32_t      load_dbgctl:1;  /* load debugctl */
      uint32_t      r2:6;
      uint32_t      ia32e:1;        /* ia32e mode on vm entry */
      uint32_t      smm:1;          /* smm mode on vm entry */
      uint32_t      dual:1;         /* treatment of smm and smi */
      uint32_t      r3:1;
      uint32_t      load_ia32_perf:1;
      uint32_t      load_ia32_pat:1;
      uint32_t      load_ia32_efer:1;
      uint32_t      r4:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_entry_ctl_vect_t;

/*
** VMCS VM entry interruption information field
*/
typedef union vmcs_entry_control_interruption_information
{
   struct
   {
      uint32_t    vector:8;   /* int/excp vector */
      uint32_t    type:3;     /* intr type  (cf. idt vectoring) */
      uint32_t    dec:1;      /* deliver error code (1) */
      uint32_t    r:19;       /* reserved */
      uint32_t    v:1;        /* valid */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_entry_ctl_int_info_t;


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

typedef union vmcs_exit_information_exit_reason
{
   struct
   {
      uint32_t    basic:16;  /* exit reason */
      uint32_t    r1:12;
      uint32_t    mtf:1;     /* pending MTF */
      uint32_t    root:1;    /* vmx-root */
      uint32_t    r2:1;
      uint32_t    entry:1;   /* (1) vm-entry fail */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_exit_info_exit_reason_t;


/*
** VMCS VM exit information interrupt information
*/
#define VMCS_INT_INFO_TYPE_HW_INT    0
#define VMCS_INT_INFO_TYPE_RES1      1
#define VMCS_INT_INFO_TYPE_NMI       2
#define VMCS_INT_INFO_TYPE_HW_EXCP   3
#define VMCS_INT_INFO_TYPE_RES2      4
#define VMCS_INT_INFO_TYPE_RES3      5
#define VMCS_INT_INFO_TYPE_SW_EXCP   6
#define VMCS_INT_INFO_TYPE_RES4      7

typedef union vmcs_exit_information_interrupt_information
{
   struct
   {
      uint32_t    vector:8;   /* int/excp vector */
      uint32_t    type:3;     /* interrupt type  */
      uint32_t    v_err:1;    /* error code valid (1) */
      uint32_t    nmi:1;      /* nmi unblocking due to iret */
      uint32_t    r:18;       /* reserved (cleared to 0) */
      uint32_t    v:1;        /* valid */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_exit_info_int_info_t;



/*
** VMCS VM exit information IDT vectoring information
*/
#define VMCS_IDT_INFO_TYPE_HW_INT    0
#define VMCS_IDT_INFO_TYPE_RES1      1
#define VMCS_IDT_INFO_TYPE_NMI       2
#define VMCS_IDT_INFO_TYPE_HW_EXCP   3
#define VMCS_IDT_INFO_TYPE_SW_INT    4
#define VMCS_IDT_INFO_TYPE_PS_EXCP   5
#define VMCS_IDT_INFO_TYPE_SW_EXCP   6
#define VMCS_IDT_INFO_TYPE_RES2      7

typedef union vmcs_exit_information_idt_vectoring_information
{
   struct
   {
      uint32_t    vector:8;   /* int/excp vector */
      uint32_t    type:3;     /* interrupt type  */
      uint32_t    v_err:1;    /* error code valid (1) */
      uint32_t    r1:1;       /* undefined */
      uint32_t    r:18;       /* reserved (cleared to 0) */
      uint32_t    v:1;        /* valid */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_exit_info_idt_vect_t;

/*
** VMCS vm exit information instruction information
*/
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SCALING_NONE  0
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SCALING_2     1
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SCALING_4     2
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SCALING_8     3

#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_RAX       0
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_RCX       1
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_RDX       2
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_RBX       3
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_RSP       4
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_RBP       5
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_RSI       6
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_RDI       7
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_R8        8
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_R9        9
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_R10      10
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_R11      11
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_R12      12
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_R13      13
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_R14      14
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_REG_R15      15

#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_ADDR_SZ_16    0
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_ADDR_SZ_32    1
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_ADDR_SZ_64    2

#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_OP_SZ_16      0
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_OP_SZ_32      1

#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_ES    0
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_CS    1
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_SS    2
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_DS    3
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_FS    4
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_SEG_REG_GS    5

#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_TYPE_SGDT     0
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_TYPE_SIDT     1
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_TYPE_LGDT     2
#define VMCS_VM_EXIT_INFORMATION_VMX_INSN_INFORMATION_TYPE_LIDT     3

typedef union vmcs_exit_information_vmexit_insn_io
{
   struct
   {
      uint32_t    r1:7;    /* undefined */
      uint32_t    addr:3;  /* addr size */
      uint32_t    r2:5;    /* undefined */
      uint32_t    seg:3;   /* seg reg   */
      uint32_t    r3:14;   /* undefined */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_exit_info_insn_io_t;

typedef union vmcs_exit_information_vmexit_insn_descriptor_table
{
   struct
   {
      uint32_t    scale:2;   /* scaling      */
      uint32_t    r1:5;      /* undefined    */
      uint32_t    addr:3;    /* addr size    */
      uint32_t    r2:1;      /* 0            */
      uint32_t    op:1;      /* op size      */
      uint32_t    r3:3;      /* undefined    */
      uint32_t    seg:3;     /* seg reg      */
      uint32_t    idx:4;     /* idx reg      */
      uint32_t    no_idx:1;  /* idx invalid  */
      uint32_t    base:4;    /* base reg     */
      uint32_t    no_base:1; /* base invalid */
      uint32_t    type:2;    /* dt type      */
      uint32_t    r4:2;      /* undefined    */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_exit_info_insn_dt_t;

typedef union vmcs_exit_information_vmexit_instruction_information
{
   raw32_t;
   vmcs_exit_info_insn_io_t io;  /* ins/outs specific info */
   vmcs_exit_info_insn_dt_t dt;  /* lgdt/lidt/sgdt/sidt */

} __attribute__((packed)) vmcs_exit_info_insn_t;


/*
** Qualification exit reason
*/

/* qualification for control register accesses */
#define VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_MOV_T_CR  0
#define VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_MOV_F_CR  1
#define VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_CLTS      2
#define VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_LMSW      3

typedef union vmcs_exit_information_qualification_cr
{
   struct
   {
      uint64_t    nr:4;          /* number of control register */
      uint64_t    type:2;        /* access type */
      uint64_t    lmsw_op:1;     /* (0) register, (1) memory */
      uint64_t    r1:1;          /* reserved */
      uint64_t    gpr:4;         /* mov cr GPR, cf. VMX_INSN_INFORMATION_REG */
      uint64_t    r2:4;          /* reserved */
      uint64_t    lmsw_data:16;  /* source data of lmsw */
      uint64_t    r3:32;         /* reserved */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcs_exit_info_cr_t;

typedef union vmcs_exit_information_qualification_dr
{
   struct
   {
      uint64_t    nr:3;          /* number of debug register */
      uint64_t    r1:1;          /* reserved */
      uint64_t    dir:1;         /* direction (0) mov to, (1) mov from */
      uint64_t    r2:3;          /* reserved */
      uint64_t    gpr:4;         /* used GRP */
      uint64_t    r3:52;         /* reserved */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcs_exit_info_dr_t;

#define VMCS_VM_EXIT_INFORMATION_QUALIFICATION_IO_OP_SZ_1B   0
#define VMCS_VM_EXIT_INFORMATION_QUALIFICATION_IO_OP_SZ_2B   1
#define VMCS_VM_EXIT_INFORMATION_QUALIFICATION_IO_OP_SZ_4B   3

typedef union vmcs_exit_information_qualification_io_insn
{
   struct
   {
      uint64_t    sz:3;          /* operand size 1/2/4 (0,1,3) */
      uint64_t    d:1;           /* in (1) or out (0) */
      uint64_t    s:1;           /* string operation */
      uint64_t    rep:1;         /* REP prefix */
      uint64_t    op:1;          /* operand encoding (0) DX, (1) imm */
      uint64_t    r1:9;          /* reserved */
      uint64_t    port:16;       /* port number */
      uint64_t    r2:32;         /* reserved */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcs_exit_info_io_t;

typedef vmcs_exit_info_io_t vmx_io_t;

/* qualification for IO insns */
typedef union vmcs_exit_information_qualification
{
   raw64_t;
   vmcs_exit_info_cr_t cr;
   vmcs_exit_info_io_t io;
   vmcs_exit_info_dr_t dr;

} __attribute__((packed)) vmcs_exit_info_qualif_t;

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
   vmcs_t(raw32_t,                        int_err_code);
   
   /* during event delivery */ 
   vmcs_t(vmcs_exit_info_idt_vect_t,      idt_info);
   vmcs_t(raw32_t,                        idt_err_code);

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

#define VMCS_CPU_REGION_SZ  0x1000

typedef union vmcs_cpu_region
{
   vmcs_region_t;
   uint8_t raw[VMCS_CPU_REGION_SZ]; /* XXX: alloc sz is given */

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

