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

typedef raw32_t vmx_insn_err_t;

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
#define VM_INSN_ERROR_VMRW_INVL_VMCS_FIELD          12
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
void    vmx_vmlaunch();

#define vmx_vmxon(vmcs)          __vmx_vmxon(vmcs)
#define vmx_vmclear(err,vmcs)    __vmx_insn2(__vmx_vmclear,err,vmcs)
#define vmx_vmload(err,vmcs)     __vmx_insn2(__vmx_vmload,err,vmcs)
#define vmx_vmread(err,addr,enc) __vmx_insn3(__vmx_vmread,err,addr,enc)
#define vmx_vmwrite(err,val,enc) __vmx_insn3(__vmx_vmwrite,err,val,enc)

/*
** VMX insn operates on 64 bits in long mode
** so we ensure error code allocation
*/
#define __vmx_insn2(fn, err, vmcs)		\
   ({						\
      raw64_t __err;				\
      int     __ret = fn(&__err.raw, vmcs);	\
      (err)->raw = __err.low;			\
      __ret;					\
   })

#define __vmx_insn3(fn, err, x, enc)		\
   ({						\
      raw64_t __err;				\
      int     __ret = fn(&__err.raw, x, enc);	\
      (err)->raw = __err.low;			\
      __ret;					\
   })

int  __vmx_vmxon(uint64_t*)                       __regparm__(1);
int  __vmx_vmclear(uint64_t*, uint64_t*)          __regparm__(2);
int  __vmx_vmload(uint64_t*,  uint64_t*)          __regparm__(2);
int  __vmx_vmread(uint64_t*, uint64_t*, uint64_t) __regparm__(3);
int  __vmx_vmwrite(uint64_t*, uint64_t, uint64_t) __regparm__(3);

#endif
