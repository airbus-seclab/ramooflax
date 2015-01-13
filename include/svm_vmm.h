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
#ifndef __SVM_VMM_H__
#define __SVM_VMM_H__

#include <types.h>
#include <pagemem.h>
#include <svm_vmcb.h>

#define VM_RMODE_EXCP_BITMAP   (0)
#define VM_PMODE_EXCP_BITMAP   (0)

#define VMCB_CR_R_BITMAP       (0)
#define VMCB_CR_W_BITMAP       ((1<<3)|(1<<0))

#define SVM_IO_BITMAP_SIZE     (3*PAGE_SIZE)
#define SVM_MSR_BITMAP_SIZE    (2*PAGE_SIZE)

/*
** Require strict alignment
*/
typedef struct svm_aligned_virtual_machine_control
{
   /* page size aligned */
   vmcb_area_t      vmm_vmcb;
   vmcb_area_t      vm_vmcb;
   uint8_t          io_bitmap[SVM_IO_BITMAP_SIZE];
   uint8_t          msr_bitmap[SVM_MSR_BITMAP_SIZE];

} __attribute__((packed)) svm_vmc_t;

/*
** Does not require alignment
*/
typedef struct svm_bazaar
{
   uint32_t asid_nr;
   raw64_t  dr_shadow[6];
   raw64_t  old_cr2;      /* used to cancel event injection */

} svm_bazaar_t;

/*
** Access to vmcb pointers
*/
#define vm_state info->vm.cpu.vmc->vm_vmcb.state_area
#define vm_ctrls info->vm.cpu.vmc->vm_vmcb.ctrls_area

/*
** Functions
*/
#ifdef __INIT__
void    svm_vmm_init();
#define svm_vmm_start(rsp,rip)						\
   asm volatile("mov %0, %%rsp ; jmp *%%rax"::"m"(rsp),"a"(rip):"memory")
#endif

#endif
