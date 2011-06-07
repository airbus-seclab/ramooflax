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
#include <vmx_exit_excp.h>
#include <vmx_exit_gp.h>
#include <vmx_exit_pf.h>
#include <emulate.h>
#include <excp.h>
#include <vmx_vmcs_acc.h>
#include <klibc.h>
#include <info_data.h>


extern info_data_t *info;


int vmx_vmexit_resolve_exception()
{
   uint32_t vector;

   vmcs_read( exit_info(info)->int_info );
   vector = (uint32_t)exit_info(info)->int_info.vector;

   return resolve_exception( vector );
}

int __vmx_vmexit_inject_exception(uint32_t vector, uint32_t error, uint32_t cr2)
{
   vmcs_entry_ctl_t *ctls = entry_ctrls(info);

   ctls->int_info.raw    = 0;
   ctls->int_info.vector = vector;
   ctls->int_info.type   = VMCS_IDT_INFORMATION_TYPE_HW_EXCP;
   ctls->int_info.v      = 1;

   switch( vector )
   {
   case PAGEFAULT_EXCEPTION:
      set_cr2( cr2 );

   case DOUBLEFAULT_EXCEPTION:
   case INVALIDTSS_EXCEPTION:
   case SEGNOTPRESENT_EXCEPTION:
   case STACKFAULT_EXCEPTION:
   case GENERALPROT_EXCEPTION:
   case ALIGNCHECK_EXCEPTION:
      ctls->int_info.dec = 1;
      ctls->err_code.raw = error & 0xff;
      vmcs_dirty( ctls->err_code );
      break;

   default:
      break;
   }

   vmcs_dirty( ctls->int_info );
   return 1;
}

