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

.globl entry
.type  entry,"function"

.globl svm_vmrun
.type  svm_vmrun,"function"

.globl svm_vmexit
.type  svm_vmexit,"function"

/*
** VMM entry point
** directly resumes
** a VM
*/
entry:

/*
** VM-entry
*/
svm_vmrun:
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
	vmload  %rax
	sti
	vmrun	%rax

/*
** VM-exit
**
** XXX: we should sub "cli", "push all"
**      cycle number (sampled via setup)
**      from eax,edx returned by "rdtsc"
**      before calling svm_vmexit_handler()
*/
svm_vmexit:
	cli
 	push	%rax
 	push	%rcx
 	push	%rdx
 	push	%rbx
	sub	$8, %rsp  /* XXX: push %rsp */
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
	mov	%edx, %edi
	shl	$32, %rdi
	mov	%eax, %edi

	xor	%rbp, %rbp
	call	svm_vmexit_handler
	jmp	svm_vmrun
