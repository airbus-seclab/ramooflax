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
#include <vmm.h>
#include <vmx_msr.h>
#include <vmx_insn.h>
#include <vmx_vm.h>
#include <klibc.h>
#include <asmutils.h>
#include <info_data.h>

char vmx_vmlaunch_failure_fmt[] = "failed (%d)\n";
char vmx_vmlaunch_failure_fnm[] = "vmlaunch";

void vmx_vmm_init(info_data_t *info)
{
   vmx_basic_info_msr_t  vmx_info;
   feat_ctl_msr_t        feat;
   cr0_reg_t             cr0, cr0_f0, cr0_f1;
   cr4_reg_t             cr4, cr4_f0, cr4_f1;
   raw64_t               vmcs_addr;

   /* check hardware support */
   if( ! vmx_supported() )
      panic( "vmx not supported" );

   read_msr_vmx_basic_info( vmx_info );

   /* fixed bit settings  */
   read_msr_vmx_cr0_fixed0( cr0_f0 );
   read_msr_vmx_cr0_fixed1( cr0_f1 );
   cr0.low = (get_cr0() & cr0_f1.low) | cr0_f0.low;

   read_msr_vmx_cr4_fixed0( cr4_f0 );
   read_msr_vmx_cr4_fixed1( cr4_f1 );
   cr4.low = (get_cr4() & cr4_f1.low) | cr4_f0.low;

   read_msr_feature_ctrl( feat );
   if( ! (feat.raw & (IA32_FEATURE_CONTROL_MSR_LOCK|IA32_FEATURE_CONTROL_MSR_VMX)) )
      panic( "vmx feature BIOS-locked: 0x%x", feat.raw );

   /* setup rev id and only that */
   info->vm.cpu.vmc->vmm_cpu_vmcs.revision_id = vmx_info.revision_id;

   set_cr0( cr0.low );
   set_cr4( cr4.low );
   enable_vmx();

   /* setup vmm pointers */
   vmcs_addr.low = (uint32_t)&info->vm.cpu.vmc->vmm_cpu_vmcs;
   vmcs_addr.high = 0;

   if( ! vmx_vmxon( &vmcs_addr.raw ) )
      panic( "failed to enter vmx root operations" );

   vmx_vm_init( info );
}

