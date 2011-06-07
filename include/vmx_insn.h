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
#ifndef __VMX_INSN_H__
#define __VMX_INSN_H__

#include <types.h>
#include <print.h>

/*
** VMCS VM exit info VM-instruction error field
** error numbers
*/
#define VM_INSN_ERROR_FAIL_INVALID                   0

#define VM_INSN_ERROR_VMCALL_IN_VMXROOT              1
#define VM_INSN_ERROR_VMCLEAR_INVL_PHYS              2
#define VM_INSN_ERROR_VMCLEAR_VMXON                  3
#define VM_INSN_ERROR_VMLAUNCH_NON_CLEAR_VMCS        4
#define VM_INSN_ERROR_VMRESUME_NON_LAUNCH_VMCS       5
#define VM_INSN_ERROR_VMRESUME_CORRUPT_VMCS          6
#define VM_INSN_ERROR_VMENTRY_INVL_CTL               7
#define VM_INSN_ERROR_VMENTRY_INVL_HOST              8
#define VM_INSN_ERROR_VMPTRLD_INVL_PHYS              9
#define VM_INSN_ERROR_VMPTRLD_VMXON                 10
#define VM_INSN_ERROR_VMPTRLD_INVL_REV              11
#define VM_INSN_ERROR_VMRW_INVL_VMCS                12
#define VM_INSN_ERROR_VMWRITE_RO_VMCS               13
#define VM_INSN_ERROR_VMXON_IN_VMXROOT              15
#define VM_INSN_ERROR_VMENTRY_INVL_EXEC_VMCS        16
#define VM_INSN_ERROR_VMENTRY_NON_LAUNCH_EXEC_VMCS  17
#define VM_INSN_ERROR_VMENTRY_EXEC_VMCS_NO_VMXON    18
#define VM_INSN_ERROR_VMCALL_NON_CLEAR_VMCS         19
#define VM_INSN_ERROR_VMCALL_INVL_EXIT              20
#define VM_INSN_ERROR_VMCALL_INVL_MSEG_REV          22
#define VM_INSN_ERROR_VMXOFF_DUAL                   23
#define VM_INSN_ERROR_VMCALL_INVL_SMM               24
#define VM_INSN_ERROR_VMENTRY_INVL_EXEC             25
#define VM_INSN_ERROR_VMENTRY_EVENTS                26

/*
** Functions
*/
uint32_t  vmx_vmxon(uint64_t*)                __attribute__ ((regparm(1)));
uint32_t  vmx_vmclear(uint32_t*, uint64_t*)   __attribute__ ((regparm(2)));
uint32_t  vmx_vmload(uint32_t*, uint64_t*)    __attribute__ ((regparm(2)));
void      vmx_vmlaunch();

uint32_t  __vmx_vmwrite(uint32_t*, uint32_t, uint32_t) __attribute__ ((regparm(3)));
uint32_t  __vmx_vmread(uint32_t*, uint32_t*, uint32_t) __attribute__ ((regparm(3)));

#define   __vmx_insn_debug(_insn_,_part_,_data_,_enc_)			\
   ({ uint32_t vmx_err;							\
      if( ! __vmx_##_insn_(&vmx_err,(_data_),(_enc_)) )			\
	 panic( "failure " #_insn_ " " #_part_ " " #_enc_ " !\n");	\
   })

#define   vmx_vmread(_pArT_,_pTr_,_eNc_)      __vmx_insn_debug(vmread,_pArT_,(_pTr_),_eNc_)
#define   vmx_vmwrite(_pArT_,_daTa_,_eNc_)    __vmx_insn_debug(vmwrite,_pArT_,(_daTa_),_eNc_)

#endif

