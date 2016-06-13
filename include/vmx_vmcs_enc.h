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
#ifndef __VMX_VMCS_ENC_H__
#define __VMX_VMCS_ENC_H__

#include <types.h>

/*
** vmcs complete field encoding
*/
#define VMCS_FIELD_ENC_ACCESS_TYPE_FULL    0
#define VMCS_FIELD_ENC_ACCESS_TYPE_HIGH    1

#define VMCS_FIELD_ENC_FIELD_TYPE_CTL      0
#define VMCS_FIELD_ENC_FIELD_TYPE_RO       1
#define VMCS_FIELD_ENC_FIELD_TYPE_G_STATE  2
#define VMCS_FIELD_ENC_FIELD_TYPE_H_STATE  3

#define VMCS_FIELD_ENC_FIELD_WIDTH_16      0
#define VMCS_FIELD_ENC_FIELD_WIDTH_64      1
#define VMCS_FIELD_ENC_FIELD_WIDTH_32      2
#define VMCS_FIELD_ENC_FIELD_WIDTH_NAT     3

typedef union vmcs_field_encoding
{
   struct
   {
      uint32_t    atype:1;   /* 0      access type */
      uint32_t    index:9;   /* 1-9    index */
      uint32_t    ftype:2;   /* 10-11  field type */
      uint32_t    r1:1;      /* 12     reserved (0) */
      uint32_t    fwidth:2;  /* 13-14  field width */

      /* we use reserved bits for vmcs synchro */

      uint32_t    read:1;    /* 15 (0) need vmread  */
      uint32_t    dirty:1;   /* 16 (1) need vmwrite */
      uint32_t    fake:1;    /* 17 (1) fake vmcs field */

      uint32_t    r2:14;     /* 18-31  reserved (0) */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vmcs_field_enc_t;


/*
** 16-bit fields
*/
#define VMCS_FIELD_ENC_EXEC_CTRL_VPID                      0

#define VMCS_FIELD_ENC_GUEST_STATE_ES_SEL              0x800
#define VMCS_FIELD_ENC_GUEST_STATE_CS_SEL              0x802
#define VMCS_FIELD_ENC_GUEST_STATE_SS_SEL              0x804
#define VMCS_FIELD_ENC_GUEST_STATE_DS_SEL              0x806
#define VMCS_FIELD_ENC_GUEST_STATE_FS_SEL              0x808
#define VMCS_FIELD_ENC_GUEST_STATE_GS_SEL              0x80a
#define VMCS_FIELD_ENC_GUEST_STATE_LDTR_SEL            0x80c
#define VMCS_FIELD_ENC_GUEST_STATE_TR_SEL              0x80e

#define VMCS_FIELD_ENC_HOST_STATE_ES_SEL               0xc00
#define VMCS_FIELD_ENC_HOST_STATE_CS_SEL               0xc02
#define VMCS_FIELD_ENC_HOST_STATE_SS_SEL               0xc04
#define VMCS_FIELD_ENC_HOST_STATE_DS_SEL               0xc06
#define VMCS_FIELD_ENC_HOST_STATE_FS_SEL               0xc08
#define VMCS_FIELD_ENC_HOST_STATE_GS_SEL               0xc0a
#define VMCS_FIELD_ENC_HOST_STATE_TR_SEL               0xc0c

/*
** 64-bit fields
*/
#define VMCS_FIELD_ENC_EXEC_CTRL_ADDR_IO_MAP_A         0x2000
#define VMCS_FIELD_ENC_EXEC_CTRL_ADDR_IO_MAP_B         0x2002
#define VMCS_FIELD_ENC_EXEC_CTRL_ADDR_MSR_MAP          0x2004
#define VMCS_FIELD_ENC_EXIT_CTRL_MSR_STORE_ADDR        0x2006
#define VMCS_FIELD_ENC_EXIT_CTRL_MSR_LOAD_ADDR         0x2008
#define VMCS_FIELD_ENC_ENTRY_CTRL_MSR_LOAD_ADDR        0x200a
#define VMCS_FIELD_ENC_EXEC_CTRL_VMCS_PTR              0x200c
#define VMCS_FIELD_ENC_EXEC_CTRL_TSC_OFFSET            0x2010
#define VMCS_FIELD_ENC_EXEC_CTRL_VAPIC_PAGE_ADDR       0x2012
#define VMCS_FIELD_ENC_EXEC_CTRL_APIC_PAGE_ADDR        0x2014
#define VMCS_FIELD_ENC_EXEC_CTRL_EPTP                  0x201a

#define VMCS_FIELD_ENC_EXIT_INFO_GUEST_PHYSICAL_ADDR   0x2400

#define VMCS_FIELD_ENC_GUEST_STATE_VMCS_LINK_PTR       0x2800
#define VMCS_FIELD_ENC_GUEST_STATE_IA32_DBG_CTL        0x2802
#define VMCS_FIELD_ENC_GUEST_STATE_IA32_PAT            0x2804
#define VMCS_FIELD_ENC_GUEST_STATE_IA32_EFER           0x2806
#define VMCS_FIELD_ENC_GUEST_STATE_IA32_PERF_GLOBAL    0x2808
#define VMCS_FIELD_ENC_GUEST_STATE_PDPTE0              0x280a
#define VMCS_FIELD_ENC_GUEST_STATE_PDPTE1              0x280c
#define VMCS_FIELD_ENC_GUEST_STATE_PDPTE2              0x280e
#define VMCS_FIELD_ENC_GUEST_STATE_PDPTE3              0x2810

#define VMCS_FIELD_ENC_HOST_STATE_IA32_PAT             0x2c00
#define VMCS_FIELD_ENC_HOST_STATE_IA32_EFER            0x2c02
#define VMCS_FIELD_ENC_HOST_STATE_IA32_PERF_GLOBAL     0x2c04

/*
** 32-bit fields
*/
#define VMCS_FIELD_ENC_EXEC_CTRL_PINBASED              0x4000
#define VMCS_FIELD_ENC_EXEC_CTRL_PROCBASED             0x4002
#define VMCS_FIELD_ENC_EXEC_CTRL_EXCP_BITMAP           0x4004
#define VMCS_FIELD_ENC_EXEC_CTRL_PGF_ERR_CODE_MASK     0x4006
#define VMCS_FIELD_ENC_EXEC_CTRL_PGF_ERR_CODE_MATCH    0x4008
#define VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_COUNT      0x400a
#define VMCS_FIELD_ENC_EXIT_CTRLS                      0x400c
#define VMCS_FIELD_ENC_EXIT_CTRL_MSR_STORE_COUNT       0x400e
#define VMCS_FIELD_ENC_EXIT_CTRL_MSR_LOAD_COUNT        0x4010
#define VMCS_FIELD_ENC_ENTRY_CTRLS                     0x4012
#define VMCS_FIELD_ENC_ENTRY_CTRL_MSR_LOAD_COUNT       0x4014
#define VMCS_FIELD_ENC_ENTRY_CTRL_INT_INFO             0x4016
#define VMCS_FIELD_ENC_ENTRY_CTRL_EXCP_ERR_CODE        0x4018
#define VMCS_FIELD_ENC_ENTRY_CTRL_INSN_LEN             0x401a
#define VMCS_FIELD_ENC_EXEC_CTRL_TPR_THRESHOLD         0x401c
#define VMCS_FIELD_ENC_EXEC_CTRL_PROCBASED_2           0x401e
#define VMCS_FIELD_ENC_EXEC_CTRL_PLE_GAP               0x4020
#define VMCS_FIELD_ENC_EXEC_CTRL_PLE_WIN               0x4022

#define VMCS_FIELD_ENC_EXIT_INFO_VM_INSN_ERROR         0x4400
#define VMCS_FIELD_ENC_EXIT_INFO_REASON                0x4402
#define VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INT_INFO       0x4404
#define VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INT_ERR        0x4406
#define VMCS_FIELD_ENC_EXIT_INFO_IDT_VECT_INFO         0x4408
#define VMCS_FIELD_ENC_EXIT_INFO_IDT_VECT_ERR          0x440a
#define VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INSN_LEN       0x440c
#define VMCS_FIELD_ENC_EXIT_INFO_VMEXIT_INSN_INFO      0x440e

#define VMCS_FIELD_ENC_GUEST_STATE_ES_LIMIT            0x4800
#define VMCS_FIELD_ENC_GUEST_STATE_CS_LIMIT            0x4802
#define VMCS_FIELD_ENC_GUEST_STATE_SS_LIMIT            0x4804
#define VMCS_FIELD_ENC_GUEST_STATE_DS_LIMIT            0x4806
#define VMCS_FIELD_ENC_GUEST_STATE_FS_LIMIT            0x4808
#define VMCS_FIELD_ENC_GUEST_STATE_GS_LIMIT            0x480a
#define VMCS_FIELD_ENC_GUEST_STATE_LDTR_LIMIT          0x480c
#define VMCS_FIELD_ENC_GUEST_STATE_TR_LIMIT            0x480e
#define VMCS_FIELD_ENC_GUEST_STATE_GDTR_LIMIT          0x4810
#define VMCS_FIELD_ENC_GUEST_STATE_IDTR_LIMIT          0x4812
#define VMCS_FIELD_ENC_GUEST_STATE_ES_ACCESS_RIGHTS    0x4814
#define VMCS_FIELD_ENC_GUEST_STATE_CS_ACCESS_RIGHTS    0x4816
#define VMCS_FIELD_ENC_GUEST_STATE_SS_ACCESS_RIGHTS    0x4818
#define VMCS_FIELD_ENC_GUEST_STATE_DS_ACCESS_RIGHTS    0x481a
#define VMCS_FIELD_ENC_GUEST_STATE_FS_ACCESS_RIGHTS    0x481c
#define VMCS_FIELD_ENC_GUEST_STATE_GS_ACCESS_RIGHTS    0x481e
#define VMCS_FIELD_ENC_GUEST_STATE_LDTR_ACCESS_RIGHTS  0x4820
#define VMCS_FIELD_ENC_GUEST_STATE_TR_ACCESS_RIGHTS    0x4822
#define VMCS_FIELD_ENC_GUEST_STATE_INT_STATE           0x4824
#define VMCS_FIELD_ENC_GUEST_STATE_ACTIVITY_STATE      0x4826
#define VMCS_FIELD_ENC_GUEST_STATE_SMBASE              0x4828
#define VMCS_FIELD_ENC_GUEST_STATE_IA32_SYSENTER_CS    0x482a
#define VMCS_FIELD_ENC_GUEST_STATE_PREEMPT_TIMER       0x482e

#define VMCS_FIELD_ENC_HOST_STATE_IA32_SYSENTER_CS     0x4c00

/*
** Natural fields
*/
#define VMCS_FIELD_ENC_EXEC_CTRL_CR0_GUEST_HOST_MASK   0x6000
#define VMCS_FIELD_ENC_EXEC_CTRL_CR4_GUEST_HOST_MASK   0x6002
#define VMCS_FIELD_ENC_EXEC_CTRL_CR0_READ_SHADOW       0x6004
#define VMCS_FIELD_ENC_EXEC_CTRL_CR4_READ_SHADOW       0x6006
#define VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_0          0x6008
#define VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_1          0x600a
#define VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_2          0x600c
#define VMCS_FIELD_ENC_EXEC_CTRL_CR3_TARGET_3          0x600e

#define VMCS_FIELD_ENC_EXIT_INFO_QUALIFICATION         0x6400
#define VMCS_FIELD_ENC_EXIT_INFO_IO_RCX                0x6402
#define VMCS_FIELD_ENC_EXIT_INFO_IO_RSI                0x6404
#define VMCS_FIELD_ENC_EXIT_INFO_IO_RDI                0x6406
#define VMCS_FIELD_ENC_EXIT_INFO_IO_RIP                0x6408
#define VMCS_FIELD_ENC_EXIT_INFO_GUEST_LINEAR_ADDR     0x640a

#define VMCS_FIELD_ENC_GUEST_STATE_CR0                 0x6800
#define VMCS_FIELD_ENC_GUEST_STATE_CR3                 0x6802
#define VMCS_FIELD_ENC_GUEST_STATE_CR4                 0x6804
#define VMCS_FIELD_ENC_GUEST_STATE_ES_BASE             0x6806
#define VMCS_FIELD_ENC_GUEST_STATE_CS_BASE             0x6808
#define VMCS_FIELD_ENC_GUEST_STATE_SS_BASE             0x680a
#define VMCS_FIELD_ENC_GUEST_STATE_DS_BASE             0x680c
#define VMCS_FIELD_ENC_GUEST_STATE_FS_BASE             0x680e
#define VMCS_FIELD_ENC_GUEST_STATE_GS_BASE             0x6810
#define VMCS_FIELD_ENC_GUEST_STATE_TR_BASE             0x6814
#define VMCS_FIELD_ENC_GUEST_STATE_LDTR_BASE           0x6812
#define VMCS_FIELD_ENC_GUEST_STATE_GDTR_BASE           0x6816
#define VMCS_FIELD_ENC_GUEST_STATE_IDTR_BASE           0x6818
#define VMCS_FIELD_ENC_GUEST_STATE_DR7                 0x681a
#define VMCS_FIELD_ENC_GUEST_STATE_RSP                 0x681c
#define VMCS_FIELD_ENC_GUEST_STATE_RIP                 0x681e
#define VMCS_FIELD_ENC_GUEST_STATE_RFLAGS              0x6820
#define VMCS_FIELD_ENC_GUEST_STATE_PENDING_DBG_EXCP    0x6822
#define VMCS_FIELD_ENC_GUEST_STATE_IA32_SYSENTER_ESP   0x6824
#define VMCS_FIELD_ENC_GUEST_STATE_IA32_SYSENTER_EIP   0x6826

#define VMCS_FIELD_ENC_HOST_STATE_CR0                  0x6c00
#define VMCS_FIELD_ENC_HOST_STATE_CR3                  0x6c02
#define VMCS_FIELD_ENC_HOST_STATE_CR4                  0x6c04
#define VMCS_FIELD_ENC_HOST_STATE_FS_BASE              0x6c06
#define VMCS_FIELD_ENC_HOST_STATE_GS_BASE              0x6c08
#define VMCS_FIELD_ENC_HOST_STATE_TR_BASE              0x6c0a
#define VMCS_FIELD_ENC_HOST_STATE_GDTR_BASE            0x6c0c
#define VMCS_FIELD_ENC_HOST_STATE_IDTR_BASE            0x6c0e
#define VMCS_FIELD_ENC_HOST_STATE_IA32_SYSENTER_ESP    0x6c10
#define VMCS_FIELD_ENC_HOST_STATE_IA32_SYSENTER_EIP    0x6c12
#define VMCS_FIELD_ENC_HOST_STATE_RSP                  0x6c14
#define VMCS_FIELD_ENC_HOST_STATE_RIP                  0x6c16

/*
** Functions
*/
#ifdef __INIT__
void  vmx_vmcs_encode();
#endif

#endif
