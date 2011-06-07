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
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;
   offset_t          fault = ctrls->exit_info_2.raw;

   debug(SVM_EXCP_PF,
	 "#PF 0x%X (p:%d wr:%d us:%d rsv:%d id:%d)\n"
	 ,fault, ctrls->exit_info_1.npf.p
	 ,ctrls->exit_info_1.npf.wr, ctrls->exit_info_1.npf.us
	 ,ctrls->exit_info_1.npf.rsv, ctrls->exit_info_1.npf.id);
 
   pg_show(fault);
   return __svm_vmexit_inject_exception(PF_EXCP, ctrls->exit_info_1.low, fault);
}

int svm_vmexit_resolve_npf()
{
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;
   offset_t          addr   = ctrls->exit_info_2.raw;
   offset_t          paddr  = 0;

   if(vmm_area(addr))
      debug(SVM_NPF, "guest access into vmm area !\n");

   if(pg_nested_walk(addr, &paddr))
      debug(SVM_NPF, "guest paddr 0x%X -> system paddr 0x%X\n", addr, paddr);

   debug(SVM_NPF,
	 "#NPF 0x%X (p:%d wr:%d us:%d rsv:%d id:%d final:%d ptb:%d)\n"
	 ,addr, ctrls->exit_info_1.npf.p
	 ,ctrls->exit_info_1.npf.wr, ctrls->exit_info_1.npf.us
	 ,ctrls->exit_info_1.npf.rsv, ctrls->exit_info_1.npf.id
	 ,ctrls->exit_info_1.npf.final, ctrls->exit_info_1.npf.ptb);

   return 0;
}
