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
.text

.globl __vmx_vmwrite
.type  __vmx_vmwrite,"function"

.globl __vmx_vmread
.type  __vmx_vmread,"function"

.globl vmx_vmresume
.type  vmx_vmresume,"function"

.globl vmx_vmexit
.type  vmx_vmexit,"function"

.globl entry
.type  entry,"function"


/*
** VMM entry point
** is vm-exit handler
** (set to host rip)
*/
entry:

/*
** VM-exit
**
** XXX: we should sub "push all" cycle number
**      (sampled via setup) from rax,rdx
**      returned by "rdtsc" before calling
**      vmx_vmexit_handler()
*/
vmx_vmexit:
        push    %rax
        push    %rcx
        push    %rdx
        push    %rbx
        sub     $8, %rsp /* XXX: push %rsp */
        push    %rbp
        push    %rsi
        push    %rdi
        push    %r8
        push    %r9
        push    %r10
        push    %r11
        push    %r12
        push    %r13
        push    %r14
        push    %r15

        lfence
        rdtsc
        mov     %edx, %edi
        shl     $32, %rdi
        or      %rax, %rdi

        xor     %rbp, %rbp
        call    vmx_vmexit_handler

/*
** VM-entry
*/
vmx_vmresume:
        pop     %r15
        pop     %r14
        pop     %r13
        pop     %r12
        pop     %r11
        pop     %r10
        pop     %r9
        pop     %r8
        pop     %rdi
        pop     %rsi
        pop     %rbp
        add     $8, %rsp  /* XXX: pop %rsp */
        pop     %rbx
        pop     %rdx
        pop     %rcx
        pop     %rax
        vmresume

/*
** VM-entry failure
**
** Params:
**      RDI = mem64 VMX error code ptr = @vmx_err
*/
__vmx_vmresume_failure_wrapper:
        sub     $8, %rsp
        mov     %rsp, %rdi
        call    vmx_check_error
        movl    (%rdi), %edi
        jmp     vmx_vmresume_failure

/*
** VM write
**
** params:
**      RDI = mem64 VMX error code ptr
**      RSI = value to write
**      RDX = VMCS field encoding
**
** returns:
**      0 on failure
**      1 on success
*/
__vmx_vmwrite:
        vmwrite %rsi, %rdx
        jmp     vmx_check_error

/*
** VM read
**
** params:
**      RDI = mem64 VMX error code ptr
**      RDI = mem64 read value ptr
**      RDX = VMCS field encoding
**
** returns:
**      0 on failure
**      1 on success
*/
__vmx_vmread:
        vmread  %rdx, (%rsi)
        jmp     vmx_check_error

/*
** Failure handling
*/
vmx_check_error:
        jz      vmx_fail_valid
        jc      vmx_fail_invalid
vmx_success:
        mov     $1, %rax
        ret

/*
** VM Fail Valid : ZF=1
**
** read VMCS instruction error (0x4400)
** store it to (%rdi)
*/
vmx_fail_valid:
        push    %rdx
        mov     $0x4400, %rdx
        vmread  %rdx, (%rdi)
        pop     %rdx
        jmp     vmx_fail

/*
** VM Fail Invalid : CF=1
**
** VMCS instruction error code is 0
*/
vmx_fail_invalid:
        movl    $0, (%rdi)

vmx_fail:
        xor     %rax, %rax
        ret

