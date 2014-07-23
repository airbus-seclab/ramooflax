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
#include <vmx_exit_pf.h>
#include <vmx_vmcs_acc.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

int __vmx_vmexit_resolve_pf()
{
   debug(VMX_EXCP_PF, "not implemented\n");
   return VM_FAIL;
}

int vmx_vmexit_resolve_ept_viol()
{
#ifdef VMX_EPT_DBG
   vmcs_exit_info_ept_t error;
   offset_t             gvaddr, gpaddr;

   vmcs_read(vm_exit_info.qualification);
   error.raw = vm_exit_info.qualification.raw;

   vmcs_read(vm_exit_info.guest_physical);
   gpaddr = vm_exit_info.guest_physical.raw;

   if(error.gl)
   {
      vmcs_read(vm_exit_info.guest_linear);
      gvaddr = vm_exit_info.guest_linear.raw;
   }
   else
      gvaddr = __rip.raw & 0xffffffffUL;

   debug(VMX_EPT,
	 "#NPF gv 0x%X gp 0x%X err 0x%X "
	 "details (r:%d w:%d x:%d gr:%d gw:%d gx:%d gl:%d final:%d nmi:%d)\n"
	 ,gvaddr, gpaddr, error.raw
	 ,error.r, error.w, error.x
	 ,error.gr, error.gw, error.gx
	 ,error.gl, error.final, error.nmi);
#endif

   return VM_FAIL;
}
