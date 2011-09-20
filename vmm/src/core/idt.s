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

idt_checkmode:
	bt	$8, irq_msg(%rip)
	jnc	idt_common
	btr	$9, irq_msg(%rip)
	jnc	idt_common
	sub	$16, %rsp
	mov	%rax, (%rsp)
	mov	16(%rsp), %rax
	movq	$-1, 16(%rsp)
	mov	%rax, 8(%rsp)
	jmp	idt_common_rcx

/*
** ring0 int64 stack layout
**
** +176 (22*8) SS
** +168        RSP
** +160        RFLAGS
** +152        CS
** +144        RIP
** +136        ERR CODE
** +128        INT NUMBER
** +120 (15*8) RAX
** ...         GPR(s)
** +0          R15
*/
idt_common:
	push	%rax
idt_common_rcx:
	push	%rcx
	push	%rdx
	push	%rbx
	push	%rsp
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

	mov	%rsp, %rdi
	call	intr_hdlr

resume:
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
	pop	%rsp
	pop	%rbx
	pop	%rdx
	pop	%rcx
	pop	%rax

	add	$16, %rsp
	rex.w   iret

/*
** IDT handlers
*/
.section	.idt_jmp, "ax", @progbits

idt_trampoline:
/* divide error (no) */
	.align	16
	pushq	$-1
	pushq	$0
	jmp	idt_common

/* debug (no) */
	.align	16
	pushq	$-1
	pushq	$1
	jmp	idt_common

/* nmi (no ) */
	.align	16
	pushq	$-1
	pushq	$2
	jmp	idt_common

/* breakpoint (no) */
	.align	16
	pushq	$-1
	pushq	$3
	jmp	idt_common

/* overflow (no) */
	.align	16
	pushq	$-1
	pushq	$4
	jmp	idt_common

/* bound (no) */
	.align	16
	pushq	$-1
	pushq	$5
	jmp	idt_common

/* invalid opcode (no) */
	.align	16
	pushq	$-1
	pushq	$6
	jmp	idt_common

/* device not available (no) */
	.align	16
	pushq	$-1
	pushq	$7
	jmp	idt_common

/* double fault (yes) */
	.align	16
	pushq	$8
	jmp	idt_checkmode

/* copro segment (no) */
	.align	16
	pushq	$-1
	pushq	$9
	jmp	idt_common

/* TSS invalid (yes) */
	.align	16
	pushq	$10
	jmp	idt_checkmode

/* Segment not present (yes) */
	.align	16
	pushq	$11
	jmp	idt_checkmode

/* stack segment fault (yes) */
	.align	16
	pushq	$12
	jmp	idt_checkmode

/* general protection (yes) */
	.align	16
	pushq	$13
	jmp	idt_checkmode

/* page fault (yes) */
	.align	16
	pushq	$14
	jmp	idt_checkmode

/* intel reserved */
	.align	16
	pushq	$-1
	pushq	$15
	jmp	idt_common

/* fpu (no ) */
	.align	16
	pushq	$-1
	pushq	$16
	jmp	idt_common

/* alignment (yes) */
	.align	16
	pushq	$17
	jmp	idt_checkmode

/* machine check (no) */
	.align	16
	pushq	$-1
	pushq	$18
	jmp	idt_common

/* simd (no */
	.align	16
	pushq	$-1
	pushq	$19
	jmp	idt_common

/* intel reserved 20-31 and user available 32-255 */
	.irp    nr, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
	.align	16
	pushq	$-1
 	pushq	$\nr
	jmp	idt_common
	.endr
