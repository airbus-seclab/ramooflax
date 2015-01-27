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
#ifndef __VMX_EXIT_H__
#define __VMX_EXIT_H__

#include <types.h>

/*
** VMCS VM exit information exit reason
*/
#define VMX_VMEXIT_EXCP_NMI          0
#define VMX_VMEXIT_EXT_INT           1
#define VMX_VMEXIT_TRI_FAULT         2
#define VMX_VMEXIT_INIT_SIG          3
#define VMX_VMEXIT_SIPI              4
#define VMX_VMEXIT_IO_SMI            5
#define VMX_VMEXIT_OTHER_SMI         6
#define VMX_VMEXIT_INT_WIN           7
#define VMX_VMEXIT_NMI_WIN           8
#define VMX_VMEXIT_TASK_SW           9
#define VMX_VMEXIT_CPUID            10
#define VMX_VMEXIT_GETSEC           11
#define VMX_VMEXIT_HLT              12
#define VMX_VMEXIT_INVD             13
#define VMX_VMEXIT_INVLPG           14
#define VMX_VMEXIT_RDPMC            15
#define VMX_VMEXIT_RDTSC            16
#define VMX_VMEXIT_RSM              17
#define VMX_VMEXIT_VMCALL           18
#define VMX_VMEXIT_VMCLEAR          19
#define VMX_VMEXIT_VMLAUNCH         20
#define VMX_VMEXIT_VMPTRLD          21
#define VMX_VMEXIT_VMPTRST          22
#define VMX_VMEXIT_VMREAD           23
#define VMX_VMEXIT_VMRESUME         24
#define VMX_VMEXIT_VMWRITE          25
#define VMX_VMEXIT_VMXOFF           26
#define VMX_VMEXIT_VMXON            27
#define VMX_VMEXIT_CR_ACCESS        28
#define VMX_VMEXIT_MOV_DR           29
#define VMX_VMEXIT_IO_INSN          30
#define VMX_VMEXIT_RDMSR            31
#define VMX_VMEXIT_WRMSR            32
#define VMX_VMEXIT_INVL_G_STATE     33
#define VMX_VMEXIT_MSR_LOAD         34
//undefined
#define VMX_VMEXIT_MWAIT            36
#define VMX_VMEXIT_MTF              37
//undefined
#define VMX_VMEXIT_MONITOR          39
#define VMX_VMEXIT_PAUSE            40
#define VMX_VMEXIT_MACH_CHECK       41
//undefined
#define VMX_VMEXIT_TPR              43
#define VMX_VMEXIT_APIC             44
//undefined
#define VMX_VMEXIT_DTR              46
#define VMX_VMEXIT_LDTR             47
#define VMX_VMEXIT_EPT              48
#define VMX_VMEXIT_EPT_CONF         49
#define VMX_VMEXIT_INVEPT           50
#define VMX_VMEXIT_RDTSCP           51
#define VMX_VMEXIT_PREEMPT          52
#define VMX_VMEXIT_INVVPID          53
#define VMX_VMEXIT_WBINVD           54
#define VMX_VMEXIT_XSETBV           55

#define VMX_VMEXIT_FIRST_RESOLVER      VMX_VMEXIT_EXCP_NMI
#define VMX_VMEXIT_LAST_RESOLVER       VMX_VMEXIT_XSETBV
#define VMX_VMEXIT_RESOLVERS_NR					\
   (VMX_VMEXIT_LAST_RESOLVER - VMX_VMEXIT_FIRST_RESOLVER + 1)

#define vmx_vmexit_resolve(x)						\
   ({									\
      int rc = VM_FAIL;							\
      if(x <= VMX_VMEXIT_LAST_RESOLVER)					\
	 rc = vmx_vmexit_resolvers[(x)]();				\
      rc;								\
   })

#define __vmx_vmexit_on_excp()   (vm_exit_info.reason.basic == VMX_VMEXIT_EXCP_NMI)

#define __vmx_vmexit_on_ept()				\
   (vm_exit_info.reason.basic == VMX_VMEXIT_EPT ||	\
    vm_exit_info.reason.basic == VMX_VMEXIT_EPT_CONF)

#define __vmx_vmexit_on_event()						\
   (vm_exit_info.reason.basic <= VMX_VMEXIT_NMI_WIN ||			\
    vm_exit_info.reason.basic == VMX_VMEXIT_MACH_CHECK)

#define __vmx_vmexit_on_insn()  (!__vmx_vmexit_on_ept() && !__vmx_vmexit_on_event())

/*
** Functions
*/
typedef int (*vmexit_hdlr_t)();

void  vmx_vmexit_handler(raw64_t) __regparm__(1);
int   vmx_vmexit_resolve_dispatcher();
int   vmx_vmexit_resolve_preempt();
int   vmx_vmexit_idt_deliver();
int   __vmx_vmexit_idt_deliver_rmode();
int   __vmx_vmexit_idt_deliver_pmode();

#endif
