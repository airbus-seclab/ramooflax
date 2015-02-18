/*
** Copyright (C) 2014 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#ifndef __VMX_TYPES_H__
#define __VMX_TYPES_H__

#include <types.h>

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

#define VMX_VMCS_GUEST_ACTIVITY_STATE_ACTIVE    0
#define VMX_VMCS_GUEST_ACTIVITY_STATE_HALT      1
#define VMX_VMCS_GUEST_ACTIVITY_STATE_SHUTDOWN  2
#define VMX_VMCS_GUEST_ACTIVITY_STATE_SIPI      3

typedef union vmcs_guest_interruptibility_state
{
   struct
   {
      uint32_t   sti:1;   /* blocking by STI */
      uint32_t   mss:1;   /* blocking by mov ss */
      uint32_t   smi:1;   /* blocking by smi */
      uint32_t   nmi:1;   /* blocking by nmi */
      uint32_t   rsv:28;  /* reserved, must be set to 0 on VMentry */

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

typedef struct vmcs_execution_pin_base_control_fields
{
   uint32_t   eint:1;      /* 0 ext intr cause vm exit (1) */
   uint32_t   r1:2;        /* 1-2 reserved */
   uint32_t   nmi:1;       /* 3 nmi cause vm exit (1) */
   uint32_t   r2:1;        /* 4 reserved */
   uint32_t   vnmi:1;      /* 5 virtual nmi control */
   uint32_t   preempt:1;   /* 6 enable vmx preemption timer */
   uint32_t   r3:25;       /* reserved */

} __attribute__((packed)) vmcs_exec_ctl_pin_based_fields_t;

typedef union vmcs_execution_pin_based_control
{
   vmcs_exec_ctl_pin_based_fields_t;
   raw32_t;

} __attribute__((packed)) vmcs_exec_ctl_pin_based_t;

typedef struct vmcs_primary_execution_proc_based_control_fields
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

} __attribute__((packed)) vmcs_exec_ctl_proc_based_fields_t;

typedef union vmcs_primary_execution_proc_based_control
{
   vmcs_exec_ctl_proc_based_fields_t;
   raw32_t;

} __attribute__((packed)) vmcs_exec_ctl_proc_based_t;

typedef struct vmcs_secondary_execution_proc_based_control_fields
{
   uint32_t   vapic:1;      /* 0     virtualize apic accesses */
   uint32_t   ept:1;        /* 1     enable EPT */
   uint32_t   dt:1;         /* 2     descriptor table exiting */
   uint32_t   rdtscp:1;     /* 3     rdtscp raises #UD */
   uint32_t   x2apic:1;     /* 4     virtualize x2apic mode */
   uint32_t   vpid:1;       /* 5     enable vpid */
   uint32_t   wbinvd:1;     /* 6     exit on wbinvd */
   uint32_t   uguest:1;     /* 7     unrestricted guest */
   uint32_t   vapic_reg:1;  /* 8     apic register virtualization */
   uint32_t   vintr:1;      /* 9     virtual interrupt delivery */
   uint32_t   pause_loop:1; /* 10    pause loop exiting */
   uint32_t   rdrand:1;     /* 11    exit on rdrand */
   uint32_t   invpcid:1;    /* 12    invpcid raises #UD */
   uint32_t   vmfunc:1;     /* 13    enable vm functions */
   uint32_t   vmcs_shdw:1;  /* 14    vmread/write to shadow vmcs */
   uint32_t   rsv:3;        /* 15-17 reserved */
   uint32_t   ept_ve:1;     /* 18    EPT violation raises #VE */

} __attribute__((packed)) vmcs_exec_ctl_proc2_based_fields_t;

typedef union vmcs_secondary_execution_proc_based_control
{
   vmcs_exec_ctl_proc2_based_fields_t;
   raw32_t;

} __attribute__((packed)) vmcs_exec_ctl_proc2_based_t;

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
** Event information type
** valid for entry injection, exit interrupt, idt vectoring
*/
#define VMCS_EVT_INFO_TYPE_HW_INT    0
#define VMCS_EVT_INFO_TYPE_RSV       1
#define VMCS_EVT_INFO_TYPE_NMI       2
#define VMCS_EVT_INFO_TYPE_HW_EXCP   3 /* not int3 and int0 */
#define VMCS_EVT_INFO_TYPE_SW_INT    4
#define VMCS_EVT_INFO_TYPE_PS_EXCP   5
#define VMCS_EVT_INFO_TYPE_SW_EXCP   6 /* int1, int3, into */
#define VMCS_EVT_INFO_TYPE_OTHER     7

typedef union vmcs_entry_control_interruption_information
{
   struct
   {
      uint32_t    vector:8;   /* int/excp vector */
      uint32_t    type:3;     /* intr type */
      uint32_t    dec:1;      /* deliver error code (1) */
      uint32_t    r:19;       /* reserved */
      uint32_t    v:1;        /* valid */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_entry_ctl_int_info_t;

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

typedef union vmcs_exit_information_qualification_ept_violation
{
   struct
   {
      uint64_t    r:1;      /* read access */
      uint64_t    w:1;      /* write access */
      uint64_t    x:1;      /* execute access */
      uint64_t    gr:1;     /* guest physical was readable */
      uint64_t    gw:1;     /* guest physical was writable */
      uint64_t    gx:1;     /* guest physical was executable */
      uint64_t    r0:1;
      uint64_t    gl:1;     /* guest linear field valid */
      uint64_t    final:1;  /* if gl(1), (1) if final translate, else ptbwalk */
      uint64_t    r1:3;
      uint64_t    nmi:1;    /* nmi unblocking due to iret */

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) vmcs_exit_info_ept_t;

typedef union vmcs_exit_information_qualification
{
   vmcs_exit_info_cr_t  cr;
   vmcs_exit_info_io_t  io;
   vmcs_exit_info_dr_t  dr;
   vmcs_exit_info_ept_t ept;
   raw64_t;

} __attribute__((packed)) vmcs_exit_info_qualif_t;


#endif
