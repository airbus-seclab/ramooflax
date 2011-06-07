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
#include <vmx_vmcs.h>
#include <vmx_vmm.h>
#include <vmx_guest.h>
#include <info_data.h>
#include <dev_io_ports.h>

void vmx_vmcs_init(info_data_t *info)
{
   vmx_vmcs_controls_init( info );
   vmx_vmcs_host_state_init( info );
   vmx_vmcs_guest_state_init( info );
}

void vmx_vmcs_controls_init(info_data_t *info)
{
   vmx_vmcs_exec_controls_init( info );
   vmx_vmcs_exec_controls_io_init( info );
   vmx_vmcs_exec_controls_msr_init( info );
   vmx_vmcs_exit_controls_init( info );
   vmx_vmcs_entry_controls_init( info );
}

void vmx_vmcs_exec_controls_io_init(info_data_t *info)
{
   vmcs_exec_ctl_t *ctls = exec_ctrls(info);

   vmx_deny_io_range( info->vm.cpu.vmc, PIC1_START_PORT, PIC1_END_PORT );
   vmx_deny_io_range( info->vm.cpu.vmc, PIC2_START_PORT, PIC2_END_PORT );
   vmx_deny_io_range( info->vm.cpu.vmc, COM1_START_PORT, COM1_END_PORT );
   vmx_deny_io_range( info->vm.cpu.vmc, KBD_START_PORT, KBD_END_PORT );
   vmx_deny_io( info->vm.cpu.vmc, PS2_SYS_CTRL_PORT_A );

   ctls->proc.usio = 1;

   ctls->io_bitmap_a.low = (uint32_t)info->vm.cpu.vmc->io_bitmap_a;
   ctls->io_bitmap_b.low = (uint32_t)info->vm.cpu.vmc->io_bitmap_b;
}

void vmx_vmcs_exec_controls_msr_init(info_data_t *info)
{
   vmcs_exec_ctl_t *ctls = exec_ctrls(info);

   vmx_deny_msr_rw( info->vm.cpu.vmc, IA32_MTRR_DEF_TYPE );
   vmx_deny_msr_rw_range( info->vm.cpu.vmc, IA32_MTRR_PHYSBASE0, IA32_MTRR_PHYSMASK7 );

   ctls->proc.umsr = 1;
   ctls->msr_bitmap.low = (uint32_t)info->vm.cpu.vmc->msr_bitmap;
}

void vmx_vmcs_exec_controls_init(info_data_t *info)
{
   vmcs_exec_ctl_t           *ctls;
   vmx_pinbased_ctls_msr_t   pin_msr;
   vmx_procbased_ctls_msr_t  proc_msr;
   cr0_reg_t                 cr0_f0, cr0_f1;
   cr4_reg_t                 cr4_f0, cr4_f1;

   ctls = exec_ctrls(info);

   read_msr_vmx_pinbased_ctls( pin_msr );
   read_msr_vmx_procbased_ctls( proc_msr );

   /* pin based */
   ctls->pin.eint = 1;
   ctls->pin.nmi  = 0;
   ctls->pin.vnmi = 0;

   ctls->pin.raw &= pin_msr.allowed_1_settings;
   ctls->pin.raw |= pin_msr.allowed_0_settings;

   /* proc based */
   ctls->proc.iwe   = 0;
   ctls->proc.mwait = 0;
   ctls->proc.rdpmc = 0;
   ctls->proc.cr8l  = 0;
   ctls->proc.cr8s  = 0;
   ctls->proc.tprs  = 0;
   ctls->proc.nwe   = 0;
   ctls->proc.mdr   = 0;
   ctls->proc.mon   = 0;
   ctls->proc.pause = 0;
   ctls->proc.hlt   = 1;
   ctls->proc.invl  = 1;
   ctls->proc.rdtsc = 0;
   ctls->proc.tsc   = 0;

   ctls->proc.raw &= proc_msr.allowed_1_settings;
   ctls->proc.raw |= proc_msr.allowed_0_settings;

   ctls->excp_bitmap.raw = info->vm.cpu.dflt_excp;
   ctls->pagefault_err_code_mask.raw = 0;
   ctls->pagefault_err_code_match.raw = 0;

   /* cr shadow */
   read_msr_vmx_cr0_fixed0( cr0_f0 );
   read_msr_vmx_cr0_fixed1( cr0_f1 );

   read_msr_vmx_cr4_fixed0( cr4_f0 );
   read_msr_vmx_cr4_fixed1( cr4_f1 );

   ctls->cr0_mask.low = (0UL & cr0_f1.low) | cr0_f0.low;
   ctls->cr4_mask.low = (CR4_PSE & cr4_f1.low) | cr4_f0.low;
}

void vmx_vmcs_exit_controls_init(info_data_t *info)
{
   vmcs_exit_ctl_t     *ctls;
   vmx_exit_ctls_msr_t exit_msr;

   ctls = exit_ctrls(info);

   read_msr_vmx_exit_ctls( exit_msr );

   ctls->exit.raw &= exit_msr.allowed_1_settings;
   ctls->exit.raw |= exit_msr.allowed_0_settings;

   ctls->msr_store_addr.low = (uint32_t)&info->vm.cpu.vmc->msr_exit_store;
   ctls->msr_load_addr.low = (uint32_t)&info->vm.cpu.vmc->msr_exit_load;
}

void vmx_vmcs_entry_controls_init(info_data_t *info)
{
   vmcs_entry_ctl_t     *ctls;
   vmx_entry_ctls_msr_t entry_msr;

   ctls = entry_ctrls(info);

   read_msr_vmx_entry_ctls( entry_msr );

   ctls->entry.raw &= entry_msr.allowed_1_settings;
   ctls->entry.raw |= entry_msr.allowed_0_settings;

   ctls->msr_load_addr.low = (uint32_t)&info->vm.cpu.vmc->msr_entry_load;

}

void vmx_vmcs_host_state_init(info_data_t *info)
{
   vmcs_host_t    *state;

   state = host_state(info);

   state->cr0.low = get_cr0();
   state->cr4.low = get_cr4();
   state->cr3.low = get_cr3();

   read_msr_ia32_sysenter_cs( state->ia32_sysenter_cs_msr );
   read_msr_ia32_sysenter_esp( state->ia32_sysenter_esp_msr );
   read_msr_ia32_sysenter_eip( state->ia32_sysenter_eip_msr );

   state->rsp.low = info->vmm.stack_bottom;

   /* vm-exit handler (vmm elf entry point) */
   state->rip.low = info->vmm.entry;

   /* segmentation */
   state->cs.raw = K_CODE_SELECTOR;
   state->ss.raw = K_DATA_SELECTOR;
   state->tr.raw = K_TSS_SELECTOR;

   state->ds.raw = state->es.raw = state->fs.raw = state->gs.raw = state->ss.raw;

   state->gdtr_base_addr.low = (uint32_t)info->vmm.cpu.sg->gdt;
   state->idtr_base_addr.low = (uint32_t)info->vmm.cpu.sg->idt;
}



