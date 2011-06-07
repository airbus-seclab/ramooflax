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
**	(sampled via setup) from eax,edx
**	returned by "rdtsc" before calling
**	vmx_vmexit_handler()
*/
vmx_vmexit:
 	push	%rax
 	push	%rcx
 	push	%rdx
 	push	%rbx
	sub	$8, %rsp /* XXX: push %rsp */
	push	%rbp
	push	%rsi
	push	%rdi
	push	%r8
	push	%r9
	push	%r10
	push	%r11
	push	%r12
	push	%r13
	push	%r14
	push	%r15

	rdtsc
	xor	%rbp, %rbp
	call	vmx_vmexit_handler

/*
** VM-entry
*/
vmx_vmresume:
	pop	%r15
	pop	%r14
	pop	%r13
	pop	%r12
	pop	%r11
	pop	%r10
	pop	%r9
	pop	%r8
	pop	%rdi
	pop	%rsi
	pop	%rbp
	add	$8, %rsp  /* XXX: pop %rsp */
 	pop	%rbx
 	pop	%rdx
 	pop	%rcx
 	pop	%rax
	vmresume

/*
** VM-entry failure
**
** Handle vmresume failure
**
** esp + 16 [  vmx_err     ]
** esp + 12 [  format str  ]
** esp +  8 [  func name   ]
** esp +  4 [  fake_ret @  ]
** esp +  0 [  panic @     ]
**
** Params:
**	EAX = mem32 VMX error code ptr = @vmx_err (esp+12)
*/
vmx_vmresume_failure:
	mov	%esp, %eax
	push	$vmx_vmresume_failure_fmt
	push	$vmx_vmresume_failure_fnm
	push	$0xbadc0de
	push	$__panic
	jmp	vmx_check_error

/*
** VM write
**
** params:
**	EAX = mem32 VMX error code ptr
**	EDX = value to write
**	ECX = VMCS field encoding
**
** returns:
**	0 on failure
**	1 on success
*/
__vmx_vmwrite:
	vmwrite	%edx, %ecx
	jmp	vmx_check_error

/*
** VM read
**
** params:
**	EAX = mem32 VMX error code ptr
**	EDX = mem32 read value ptr
**	ECX = VMCS field encoding
**
** returns:
**	0 on failure
**	1 on success
*/
__vmx_vmread:
	vmread	%ecx, (%edx)
	jmp	vmx_check_error

/*
** Failure handling
*/
vmx_check_error:
	jz	vmx_fail_valid
	jc	vmx_fail_invalid
vmx_success:
	mov	$1, %eax
	ret

/*
** VM Fail Valid : ZF=1
**
** read VMCS instruction error (0x4400)
** store it to (%eax)
*/
vmx_fail_valid:
	push	%ecx
	mov	$0x4400, %ecx
	vmread	%ecx, (%eax)
	pop	%ecx
	jmp	vmx_fail

/*
** VM Fail Invalid : CF=1
**
** no VMCS instruction error code
** but inform "C" error pointer
*/
vmx_fail_invalid:
	movl	$0, (%eax)

vmx_fail:
	xor	%eax, %eax
	ret

