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
#include <vmx_exit_cr.h>
#include <vmx_exit_fail.h>
#include <vmx_vmcs_acc.h>
#include <emulate.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

int vmx_vmexit_resolve_cr_access()
{
   vmcs_exit_info_t    *exit;
   vmcs_guest_t        *state;
   vmcs_exit_info_cr_t *cr_access;
   uint8_t             gpr;

   exit = exit_info(info);
   state = guest_state(info);

   vmcs_read( exit->qualification );
   cr_access = &exit->qualification.cr_access;
   gpr = 7 - (cr_access->gpr & 7);

   switch( cr_access->type )
   {
   case VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_MOV_T_CR:
      return resolve_cr( 1, gpr, cr_access->nr );
   case VMCS_VM_EXIT_INFORMATION_QUALIFICATION_CR_ACCESS_TYPE_MOV_F_CR:
      return resolve_cr( 0, cr_access->nr, gpr );
   }

   return 0;
}

uint32_t __vmx_vmexit_mov_from_cr(uint8_t cr)
{
   vmcs_exec_ctl_t *ctrls = exec_ctrls(info);

   if( cr == 0 )
   {
      vmcs_read( ctrls->cr0_mask );
      return (((~ctrls->cr0_mask.low) & __cr0.low) | (ctrls->cr0_mask.low & __cr0_shadow.low));
   }
   else if( cr == 2 )
      return get_cr2();
   else if( cr == 3 )
      return __cr3_shadow.low;
   else if( cr == 4 )
   {
      vmcs_read( ctrls->cr4_mask );
      return (((~ctrls->cr4_mask.low) & __cr4.low) | (ctrls->cr4_mask.low & __cr4_shadow.low));
   }

   return 0;
}

