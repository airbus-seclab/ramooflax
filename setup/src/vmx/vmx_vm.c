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
#include <vmx_vm.h>
#include <vmx_vmcs.h>
#include <vmx_msr.h>
#include <vmx_insn.h>
#include <klibc.h>
#include <info_data.h>

void vmx_vm_init(info_data_t *info)
{
   vmx_basic_info_msr_t  vmx_info;
   uint32_t              vmx_err;
   raw64_t               vmcs_addr;

   /* setup rev id needed by vmclear */
   read_msr_vmx_basic_info( vmx_info );
   info->vm.cpu.vmc->vm_cpu_vmcs.revision_id = info->vm.vmcs.revision_id = vmx_info.revision_id;

   vmcs_addr.low = (uint32_t)&info->vm.cpu.vmc->vm_cpu_vmcs;
   vmcs_addr.high = 0;

   /* clear vmcs launch state */
   if( ! vmx_vmclear( &vmx_err, &vmcs_addr.raw ) )
      panic( "vmclear failed (%d) !\n", vmx_err );

   /* mark current-vmcs valid/active */
   if( ! vmx_vmload( &vmx_err, &vmcs_addr.raw ) )
      panic( "vmload failed (%d) !\n", vmx_err );

   vmx_vmcs_init( info );
   vmx_vmcs_encode( info );
   vmx_vmcs_commit( info );
}
