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
#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <types.h>
#include <gpr.h>
#include <excp.h>

#define BIOS_VIDEO_INTERRUPT          0x10
#define BIOS_DISK_INTERRUPT           0x13
#define BIOS_MISC_INTERRUPT           0x15
#define BIOS_KBD_INTERRUPT            0x16
#define BIOS_BOOT_INTERRUPT           0x19

/*
** BIOS services related to MISC_INTERRUPT
*/

/* AX values */
#define BIOS_GET_SMAP                 0xe820
#define BIOS_SMAP_ID                  0x534d4150
#define BIOS_SMAP_ERROR               0x86

#define BIOS_GET_EXT_MEM_32           0xe881
#define BIOS_GET_EXT_MEM              0xe801

#define BIOS_DISABLE_A20              0x2400
#define BIOS_ENABLE_A20               0x2401
#define BIOS_STATUS_A20               0x2402
#define BIOS_SUPPORT_A20              0x2403

/* AH values */

#define BIOS_GET_BIG_MEM              0x8a
#define BIOS_OLD_GET_EXT_MEM          0x88

/*
** Stacked by cpu on ring0 irq
*/
typedef struct cpu32_ring0_context
{
   uint32_t          nr;
   excp32_err_code_t err;
   raw32_t           eip;
   raw32_t           cs;
   eflags_reg_t      eflags;

} __attribute__((packed)) cpu32_r0_ctx_t;

typedef struct cpu64_ring0_context
{
   raw64_t           nr;
   excp64_err_code_t err;
   raw64_t           rip;
   raw64_t           cs;
   rflags_reg_t      rflags;
   raw64_t           rsp;
   raw64_t           ss;

} __attribute__((packed)) cpu64_r0_ctx_t;

/*
** Stacked ring0 interrupt context
*/
typedef struct interrupt32_ring0_context
{
   gpr32_ctx_t;
   cpu32_r0_ctx_t;

} __attribute__((packed)) int32_r0_ctx_t;

typedef struct interrupt64_ring0_context
{
   gpr64_ctx_t;
   cpu64_r0_ctx_t;

} __attribute__((packed)) int64_r0_ctx_t;

/*
+** Simple information passing between
+** idt handlers and standard code
+*/
typedef union irq_message
{
   struct
   {
      uint16_t  vector:8;
      uint16_t  preempt:1;
      uint16_t  rmode:1;

   } __attribute__((packed));

   uint16_t raw;

} __attribute__((packed)) irq_msg_t;

/*
** Interrupt Service Routine prototype
*/
typedef void (*isr32_t)(int32_r0_ctx_t*);
typedef void (*isr64_t)(int64_r0_ctx_t*);

#ifdef __INIT__
void  intr_init();
#else

#ifdef __SVM__
#include <svm_exit_int.h>
#else
#include <vmx_exit_int.h>
#endif

int   resolve_intr();
void  intr_hdlr(int64_r0_ctx_t*) __regparm__(1);

#endif

#endif
