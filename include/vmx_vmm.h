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

#include <excp.h>

#define VM_RMODE_EXCP_BITMAP   ((1<<DF_EXCP)|(1<<GP_EXCP))
#define VM_PMODE_EXCP_BITMAP   (0)

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
   vmcs_region_t  vmcs;
   uint8_t        last_pending;
   raw64_t        cr3_shadow;
   raw64_t        int_shadow;

} vmx_bazaar_t;

/*
** Fast access to vmcs pointers
*/
#define guest_state(info)  ((vmcs_guest_t*)&info->vm.vmcs.guest_state)
#define host_state(info)   ((vmcs_host_t*)&info->vm.vmcs.host_state)
#define exec_ctrls(info)   ((vmcs_exec_ctl_t*)&info->vm.vmcs.controls.exec_ctrls)
#define exit_ctrls(info)   ((vmcs_exit_ctl_t*)&info->vm.vmcs.controls.exit_ctrls)
#define entry_ctrls(info)  ((vmcs_entry_ctl_t*)&info->vm.vmcs.controls.entry_ctrls)
#define exit_info(info)    ((vmcs_exit_info_t*)&info->vm.vmcs.exit_info)

/*
** Functions
*/
#ifdef __INIT__
void    vmx_vmm_init();
#define vmx_vmm_start(rsp)					\
   asm volatile("mov %0, %%rsp ; jmp vmx_vmlaunch"::"m"(rsp):"memory")
#endif

#endif
