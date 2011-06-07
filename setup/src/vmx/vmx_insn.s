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

.globl vmx_vmxon
.type  vmx_vmxon,"function"

.globl vmx_vmclear
.type  vmx_vmclear,"function"

.globl vmx_vmload
.type  vmx_vmload,"function"

.globl vmx_vmlaunch
.type  vmx_vmlaunch,"function"


/*
** VMX insn error checks
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

/*
** VM-entry
*/
vmx_vmlaunch:
	popq	%r15
	popq	%r14
	popq	%r13
	popq	%r12
	popq	%r11
	popq	%r10
	popq	%r9
	popq	%r8
	popq	%rdi
	popq	%rsi
	popq	%rbp
	addq	$8, %rsp
 	popq	%rbx
 	popq	%rdx
 	popq	%rcx
 	popq	%rax
	vmlaunch

/*
** VM-entry failure
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
vmx_vmlaunch_failure:
	mov	%esp, %eax
	push	$vmx_vmlaunch_failure_fmt
	push	$vmx_vmlaunch_failure_fnm
	push	$0xbadc0de
	push	$__panic
	jmp	vmx_check_error

/*
** Enter VMX root operations
**
** params:
**	EAX = mem64 VMXON region paddr
**
** returns:
**	0 on failure
**	1 on success
*/
vmx_vmxon:
	vmxon	(%eax)
	jc	vmx_fail
	jmp	vmx_success

/*
** Clear VMCS
**
** params:
**	EAX = mem32 VMX error code ptr
**	EDX = mem64 VMCS region paddr
**
** returns:
**	0 on failure
**	1 on success
*/
vmx_vmclear:
	vmclear (%edx)
	jmp	vmx_check_error

/*
** Load VMCS
**
** params:
**	EAX = mem32 VMX error code ptr
**	EDX = mem64 VMCS region paddr
**
** returns:
**	0 on failure
**	1 on error
*/
vmx_vmload:
	vmptrld	(%edx)
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
