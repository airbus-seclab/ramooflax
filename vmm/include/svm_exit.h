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
#ifndef __SVM_EXIT_H__
#define __SVM_EXIT_H__

#include <types.h>

/*
** SVM Intercept Exit Codes
*/
#define SVM_VMEXIT_CR_READ_START      0
#define SVM_VMEXIT_CR_READ_END       15

#define SVM_VMEXIT_CR_WRITE_START    16
#define SVM_VMEXIT_CR_WRITE_END      31

#define SVM_VMEXIT_DR_READ_START     32
#define SVM_VMEXIT_DR_READ_END       47

#define SVM_VMEXIT_DR_WRITE_START    48
#define SVM_VMEXIT_DR_WRITE_END      63

#define SVM_VMEXIT_EXCP_START        64
#define SVM_VMEXIT_EXCP_END          95

#define SVM_VMEXIT_INTR              96
#define SVM_VMEXIT_NMI               97
#define SVM_VMEXIT_SMI               98
#define SVM_VMEXIT_INIT              99
#define SVM_VMEXIT_VINTR            100

#define SVM_VMEXIT_CR0_SEL_WRITE    101
#define SVM_VMEXIT_IDTR_READ        102
#define SVM_VMEXIT_GDTR_READ        103
#define SVM_VMEXIT_LDTR_READ        104
#define SVM_VMEXIT_TR_READ          105
#define SVM_VMEXIT_IDTR_WRITE       106
#define SVM_VMEXIT_GDTR_WRITE       107
#define SVM_VMEXIT_LDTR_WRITE       108
#define SVM_VMEXIT_TR_WRITE         109
#define SVM_VMEXIT_RDTSC            110
#define SVM_VMEXIT_RDPMC            111
#define SVM_VMEXIT_PUSHF            112
#define SVM_VMEXIT_POPF             113
#define SVM_VMEXIT_CPUID            114
#define SVM_VMEXIT_RSM              115
#define SVM_VMEXIT_IRET             116
#define SVM_VMEXIT_SWINT            117
#define SVM_VMEXIT_INVD             118
#define SVM_VMEXIT_PAUSE            119
#define SVM_VMEXIT_HLT              120
#define SVM_VMEXIT_INVLPG           121
#define SVM_VMEXIT_INVLPGA          122
#define SVM_VMEXIT_IOIO             123
#define SVM_VMEXIT_MSR              124

#define SVM_VMEXIT_TASK             125
#define SVM_VMEXIT_FERR             126
#define SVM_VMEXIT_SHUTDOWN         127

#define SVM_VMEXIT_VMRUN            128
#define SVM_VMEXIT_VMMCALL          129
#define SVM_VMEXIT_VMLOAD           130
#define SVM_VMEXIT_VMSAVE           131
#define SVM_VMEXIT_STGI             132
#define SVM_VMEXIT_CLGI             133
#define SVM_VMEXIT_SKINIT           134
#define SVM_VMEXIT_RDTSCP           135
#define SVM_VMEXIT_ICEBP            136

#define SVM_VMEXIT_WBINVD           137
#define SVM_VMEXIT_MONITOR          138
#define SVM_VMEXIT_MWAIT            139
#define SVM_VMEXIT_MWAIT_COND       140

#define SVM_VMEXIT_NPF              1024
#define SVM_VMEXIT_INVALID          -1    /* invalid guest state in vmcb */

/*
** vmexit handlers indexes
*/
#define SVM_VMEXIT_FIRST_RESOLVER      SVM_VMEXIT_INTR
#define SVM_VMEXIT_LAST_RESOLVER       SVM_VMEXIT_MWAIT_COND
#define SVM_VMEXIT_RESOLVERS_NR                                 \
   (SVM_VMEXIT_LAST_RESOLVER - SVM_VMEXIT_FIRST_RESOLVER + 1)

#define svm_vmexit_resolve(x)                                   \
   (svm_vmexit_resolvers[(x) - SVM_VMEXIT_FIRST_RESOLVER]())

#define svm_set_vmexit_resolver(x,f)                                    \
   (svm_vmexit_resolvers[(x) - SVM_VMEXIT_FIRST_RESOLVER] = (f))

#define svm_vmexit_resolver_string(x)                                   \
   (svm_vmexit_resolver_string[(x) - SVM_VMEXIT_FIRST_RESOLVER])

#define __svm_vmexit_on_excp()                                          \
   range(vm_ctrls.exit_code.low, SVM_VMEXIT_EXCP_START, SVM_VMEXIT_EXCP_END)

#define __svm_vmexit_on_event()                                         \
   (range(vm_ctrls.exit_code.low, SVM_VMEXIT_EXCP_START, SVM_VMEXIT_VINTR) || \
    range(vm_ctrls.exit_code.low, SVM_VMEXIT_TASK, SVM_VMEXIT_SHUTDOWN)    || \
    vm_ctrls.exit_code.low == SVM_VMEXIT_NPF)

#define __svm_vmexit_on_insn()    (!__svm_vmexit_on_event())

/*
** Functions
*/
typedef int (*vmexit_hdlr_t)();

void  svm_vmexit_handler(raw64_t) __regparm__(1);
int   svm_vmexit_resolve_dispatcher();
int   svm_vmexit_idt_deliver();
int   __svm_vmexit_idt_deliver_rmode();
int   __svm_vmexit_idt_deliver_pmode();

#endif
