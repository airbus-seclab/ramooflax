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
#include <pf.h>
#include <vmx_vmcs_acc.h>
#include <info_data.h>

extern info_data_t *info;

int __vmx_vmexit_resolve_pf()
{
   uint32_t         fault;
   pf_err_t         err;
   vmcs_exit_info_t *exit;

   exit = exit_info(info);

   vmcs_read( exit->qualification );
   vmcs_read( exit->int_err_code );

   err.raw = exit->int_err_code.raw;
   fault   = exit->qualification.low;

   return resolve_pf( fault, err );
}

int vmx_vmexit_resolve_invlpg()
{
   vmcs_exit_info_t *exit  = exit_info(info);
   vmcs_guest_t     *state = guest_state(info);

   exit = exit_info(info);
   vmcs_read( exit->qualification );

   if( ! __pf_invlpg( exit->qualification.low ) )
      return 0;

   vmcs_read( exit->vmexit_insn_len );
   state->rip.low += exit->vmexit_insn_len.raw;
   vmcs_dirty( state->rip );

   return 1;
}

void __vmx_update_asid()
{
}
