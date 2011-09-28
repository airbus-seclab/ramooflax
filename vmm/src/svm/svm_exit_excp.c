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
#include <svm_exit_excp.h>
#include <svm_exit_pf.h>
#include <svm_exit.h>
#include <emulate.h>
#include <excp.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

int __svm_vmexit_inject_exception(uint32_t vector, uint32_t error, uint64_t cr2)
{
   __svm_prepare_event_injection(vm_ctrls.event_injection,
				 VMCB_IDT_DELIVERY_TYPE_EXCP,
				 vector);
   switch(vector)
   {
   case PF_EXCP:
      vm_state.cr2.raw = cr2;
   case DF_EXCP:
   case TS_EXCP:
   case NP_EXCP:
   case SS_EXCP:
   case GP_EXCP:
   case AC_EXCP:
      vm_ctrls.event_injection.ev = 1;
      vm_ctrls.event_injection.err_code = error;
      break;

   default:
      break;
   }

   debug(SVM_EXCP, "inject exception #%d err 0x%x\n", vector, error);
   return 1;
}
