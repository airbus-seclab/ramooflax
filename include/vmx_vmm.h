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
#ifndef __VMX_VMM_H__
#define __VMX_VMM_H__

#include <types.h>
#include <pagemem.h>
#include <vmx_vmcs.h>
#include <vmx_msr.h>
#include <excp.h>

#define VM_RMODE_EXCP_BITMAP   (1<<GP_EXCP|1<<MC_EXCP)
#define VM_PMODE_EXCP_BITMAP   (1<<MC_EXCP)

/*
** Require strict alignment
*/
typedef struct vmx_aligned_virtual_machine_control
{
   /* page size aligned */
   vmcs_cpu_region_t          vmm_cpu_vmcs;
   vmcs_cpu_region_t          vm_cpu_vmcs;
   uint8_t                    io_bitmap_a[PAGE_SIZE];
   uint8_t                    io_bitmap_b[PAGE_SIZE];
   uint8_t                    msr_bitmap[PAGE_SIZE];

   /* 16 bytes aligned  */
   vmcs_ctl_msr_area_entry_t  msr_exit_store[PAGE_SIZE/2];
   vmcs_ctl_msr_area_entry_t  msr_exit_load[PAGE_SIZE/2];
   vmcs_ctl_msr_area_entry_t  msr_entry_load[PAGE_SIZE/2];

} __attribute__((packed)) vmx_vmc_t;

/*
** Does not require alignment
*/
typedef struct vmx_bazaar
{
   vmcs_region_t        vmcs;
   raw64_t              int_shadow;
   raw64_t              dr_shadow[6];
   offset_t             max_paddr;
   size_t               lbr_tos;
   uint16_t             idt_limit;
   ia32_mtrr_cap_t      mtrr_cap;
   ia32_mtrr_def_t      mtrr_def;
   ia32_efer_msr_t      efer;

   cr0_reg_t            cr0_dft_mask;
   cr4_reg_t            cr4_dft_mask;

   vmx_basic_info_msr_t vmx_info;
   vmx_pin_ctls_msr_t   vmx_fx_pin;
   vmx_proc_ctls_msr_t  vmx_fx_proc;
   vmx_proc_ctls_msr_t  vmx_fx_proc2;
   vmx_exit_ctls_msr_t  vmx_fx_exit;
   vmx_entry_ctls_msr_t vmx_fx_entry;
   vmx_fixed_cr_msr_t   vmx_fx_cr0;
   vmx_fixed_cr_msr_t   vmx_fx_cr4;
   vmx_ept_cap_msr_t    vmx_ept_cap;

} vmx_bazaar_t;

/*
** Access to vmcs pointers
*/
#define vm_state        info->vm.vmcs.guest_state
#define vm_host_state   info->vm.vmcs.host_state
#define vm_exec_ctrls   info->vm.vmcs.controls.exec_ctrls
#define vm_exit_ctrls   info->vm.vmcs.controls.exit_ctrls
#define vm_entry_ctrls  info->vm.vmcs.controls.entry_ctrls
#define vm_exit_info    info->vm.vmcs.exit_info

#define vmx_cpl         vm_state.cs.selector.rpl

/*
** Functions
*/
#ifdef __INIT__
void    vmx_vmm_init();
#define vmx_vmm_start(rsp)						\
   asm volatile("mov %0, %%rsp ; jmp vmx_vmlaunch"::"m"(rsp):"memory")
#endif

#endif
