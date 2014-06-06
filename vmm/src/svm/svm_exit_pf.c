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
#include <svm_exit_pf.h>
#include <paging.h>
#include <emulate.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

int __svm_vmexit_resolve_pf()
{
   offset_t fault = vm_ctrls.exit_info_2.raw;

   debug(SVM_EXCP_PF,
	 "#PF 0x%X (p:%d wr:%d us:%d rsv:%d id:%d)\n"
	 ,fault, vm_ctrls.exit_info_1.npf.p
	 ,vm_ctrls.exit_info_1.npf.wr, vm_ctrls.exit_info_1.npf.us
	 ,vm_ctrls.exit_info_1.npf.rsv, vm_ctrls.exit_info_1.npf.id);

   pg_show(fault);
   return __svm_vmexit_inject_exception(PF_EXCP, vm_ctrls.exit_info_1.low, fault);
}

int svm_vmexit_resolve_npf()
{
   vmcb_exit_info_npf_t error;
   offset_t             gvaddr, gpaddr;

   error.raw = vm_ctrls.exit_info_1.raw;
   gpaddr    = vm_ctrls.exit_info_2.raw;
   gvaddr    = __rip.raw & 0xffffffffUL;

   debug(SVM_NPF,
	 "#NPF gv 0x%X gp 0x%X err 0x%X "
	 "details (p:%d wr:%d us:%d rsv:%d id:%d final:%d ptb:%d)\n"
	 ,gvaddr, gpaddr, error.raw, error.p, error.wr, error.us
	 ,error.rsv, error.id, error.final, error.ptb);

   return VM_FAIL;
}
