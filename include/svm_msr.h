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
#ifndef __SVM_MSR_H__
#define __SVM_MSR_H__

#include <types.h>

/*
** DBG_CTL_MSR
*/
#define AMD_DBGCTL_MSR                     0x1d9UL

#define AMD_DBGCTL_LBR_BIT                 0
#define AMD_DBGCTL_BTF_BIT                 1

#define AMD_DBGCTL_LBR                     (1<<AMD_DBGCTL_LBR_BIT)
#define AMD_DBGCTL_BTF                     (1<<AMD_DBGCTL_BTF_BIT)

typedef union amd_dbgctl_msr
{
   msr_t;

   struct
   {
      uint64_t    lbr:1;    /* last branch int/excp */
      uint64_t    btf:1;    /* single step on branch */

      uint64_t    pb0:1;    /* perf monitoring pins */
      uint64_t    pb1:1;
      uint64_t    pb2:1;
      uint64_t    pb3:1;

   } __attribute__((packed));

} __attribute__((packed)) amd_dbgctl_msr_t;

#define rd_msr_amd_dbgctl(val)      rd_msr64(AMD_DBGCTL_MSR,(val).edx,(val).eax)
#define wr_msr_amd_dbgctl(val)      wr_msr64(AMD_DBGCTL_MSR,(val).edx,(val).eax)

#define set_amd_dbgctl(_n,_v)		\
   ({					\
      amd_dbgctl_msr_t dbgctl;		\
      rd_msr_amd_dbgctl(dbgctl);	\
      if(_v) dbgctl.raw |=  (_n);	\
      else   dbgctl.raw &= ~(_n);	\
      wr_msr_amd_dbgctl(dbgctl);	\
   })

#define amd_dbgctl_lbr_enable()      set_amd_dbgctl(AMD_DBGCTL_LBR, 1)
#define amd_dbgctl_lbr_disable()     set_amd_dbgctl(AMD_DBGCTL_LBR, 0)

/*
** Last Branch Record MSRs
*/
#define AMD_LBR_FROM_MSR             0x1dbUL
#define AMD_LBR_TO_MSR               0x1dcUL
#define AMD_LBR_FROM_EXCP_MSR        0x1ddUL
#define AMD_LBR_TO_EXCP_MSR          0x1deUL

#define __amd_dbgctl_lbr(_idx_)		\
   ({					\
      msr_t m;				\
      rd_msr64(_idx_, m.edx, m.eax);	\
      m.raw;				\
   })

#define amd_dbgctl_lbr_from()        __amd_dbgctl_lbr(AMD_LBR_FROM_MSR)
#define amd_dbgctl_lbr_to()          __amd_dbgctl_lbr(AMD_LBR_TO_MSR)
#define amd_dbgctl_lbr_from_excp()   __amd_dbgctl_lbr(AMD_LBR_FROM_EXCP_MSR)
#define amd_dbgctl_lbr_to_excp()     __amd_dbgctl_lbr(AMD_LBR_TO_EXCP_MSR)

/*
** AMD System Configuration Register
*/
#define AMD_SYSCFG_MSR        0xc0010010UL

#define AMD_SYSCFG_TOM2_BIT   21
#define AMD_SYSCFG_MVDM_BIT   20
#define AMD_SYSCFG_MFDM_BIT   19
#define AMD_SYSCFG_MFDE_BIT   18

#define AMD_SYSCFG_TOM2       (1<<AMD_SYSCFG_TOM2_BIT)
#define AMD_SYSCFG_MVDM       (1<<AMD_SYSCFG_MVDM_BIT)
#define AMD_SYSCFG_MFDM       (1<<AMD_SYSCFG_MFDM_BIT)
#define AMD_SYSCFG_MFDE       (1<<AMD_SYSCFG_MFDE_BIT)

typedef union amd_syscfg_msr
{
   struct
   {
      uint64_t   rsv0:18;
      uint64_t   mfde:1;
      uint64_t   mfdm:1;
      uint64_t   mvdm:1;
      uint64_t   tom2:1;

   } __attribute__((packed));

   msr_t;
   raw64_t;

} __attribute__((packed)) amd_syscfg_msr_t;

#define rd_msr_syscfg(val)      rd_msr64(AMD_SYSCFG_MSR,(val).edx,(val).eax)
#define wr_msr_syscfg(val)      wr_msr64(AMD_SYSCFG_MSR,(val).edx,(val).eax)

/*
** AMD Top Of Memory MSRs
*/
#define AMD_TOP_MEM1_MSR      0xc001001a
#define AMD_TOP_MEM2_MSR      0xc001001d

typedef union amd_top_of_memory_msr
{
   struct
   {
      uint64_t   rsv:23;
      uint64_t   addr:29; /* 8MB aligned */

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) amd_top_mem_msr_t;

#define rd_msr_topmem(_x_)		\
   ({					\
      msr_t m;		\
      rd_msr64(_x_,m.edx,m.eax);	\
      m.raw;				\
   })

#define rd_msr_topmem1()      rd_msr_topmem(AMD_TOP_MEM1_MSR)
#define rd_msr_topmem2()      rd_msr_topmem(AMD_TOP_MEM2_MSR)

/*
** Extended Feature Enable MSR : EFER MSR
*/
#define AMD_EFER_MSR          0xc0000080UL

#define AMD_EFER_LME_BIT       8
#define AMD_EFER_LMA_BIT      10
#define AMD_EFER_NXE_BIT      11
#define AMD_EFER_SVME_BIT     12

#define AMD_EFER_LME          (1<<AMD_EFER_LME_BIT)
#define AMD_EFER_LMA          (1<<AMD_EFER_LMA_BIT)
#define AMD_EFER_NXE          (1<<AMD_EFER_NXE_BIT)
#define AMD_EFER_SVME         (1<<AMD_EFER_SVME_BIT)

typedef union amd_efer_msr
{
   struct
   {
      uint64_t    sce:1;     /* syscall extensions */
      uint64_t    rsrvd0:7;  /* zero */
      uint64_t    lme:1;     /* long mode enable */
      uint64_t    rsrvd1:1;  /* zero */
      uint64_t    lma:1;     /* long mode active */
      uint64_t    nxe:1;     /* no-execute-enable */
      uint64_t    svme:1;    /* svm enable */
      uint64_t    rsvrd2:1;  /* must be zero */
      uint64_t    ffxsr:1;   /* fast fxsave/fxrstor */

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) amd_efer_msr_t;

#define rd_msr_amd_efer(val)      rd_msr64(AMD_EFER_MSR,(val).edx,(val).eax)
#define wr_msr_amd_efer(val)      wr_msr64(AMD_EFER_MSR,(val).edx,(val).eax)

#define set_amd_efer(_n,_v)		\
   ({					\
      amd_efer_msr_t efer;		\
      rd_msr_amd_efer(efer);		\
      if(_v) efer.raw |=  (_n);		\
      else   efer.raw &= ~(_n);		\
      wr_msr_amd_efer(efer);		\
   })

#define svm_enable()          set_amd_efer(AMD_EFER_SVME,1)
#define svm_disable()         set_amd_efer(AMD_EFER_SVME,0)

#define lm_enable()           set_amd_efer(AMD_EFER_LME,1)
#define lm_disable()          set_amd_efer(AMD_EFER_LME,0)
#define lm_active()           ({amd_efer_msr_t m; rd_msr_amd_efer(m); m.lma;})

/*
** VM_CR MSR
*/
#define AMD_VMCR_MSR          0xc0010114UL

typedef union amd_vmcr_msr
{
   struct
   {
      uint64_t   dpd:1;
      uint64_t   r_init:1;
      uint64_t   dis_a20m:1;
      uint64_t   lock:1;
      uint64_t   svm_dis:1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) amd_vmcr_msr_t;

#define rd_msr_vmcr(val)             rd_msr64(AMD_VMCR_MSR,(val).edx,(val).eax)
#define wr_msr_vmcr(val)             wr_msr64(AMD_VMCR_MSR,(val).edx,(val).eax)


/*
** VM_HSAVE_PA MSR
**
** Host State Block Paddr MSR
*/
#define AMD_VM_HSAVE_PA_MSR             0xc0010117UL
#define wr_msr_host_state(paddr)        wr_msr64(AMD_VM_HSAVE_PA_MSR,0,(paddr))

/*
** Interrupt Pending Message Register
*/
#define AMD_INT_PENDING_MSG_MSR         0xc0010055UL

typedef union amd_int_pending_msg
{
   struct
   {
      uint64_t      io_msg_addr:16;
      uint64_t      io_msg_data:8;
      uint64_t      int_pending_msg_dis:1;
      uint64_t      int_pending_msg:1;
      uint64_t      io_read:1;
      uint64_t      zero:37;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) amd_int_pending_msg_t;

/*
** Various msr related to vmload/vmsave insn
*/
#define AMD_STAR_MSR                0xc0000081
#define AMD_LSTAR_MSR               0xc0000082
#define AMD_CSTAR_MSR               0xc0000083
#define AMD_SFMASK_MSR              0xc0000084
#define AMD_KERNEL_GS_BASE_MSR      0xc0000102

#endif
