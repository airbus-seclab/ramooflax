/*
** Copyright (C) 2016 Airbus Group, stephane duverger <stephane.duverger@airbus.com>
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
#include <svm_guest.h>
#include <msr.h>
#include <info_data.h>

extern info_data_t *info;

void svm_vmcb_guest_state_init()
{
   vm_state.idtr.limit.wlow = 0x3ff;

   vm_state.ss.selector.raw = RM_BASE_SS;
   vm_state.ss.base.low = (vm_state.ss.selector.raw)<<4;

   vm_state.cs.limit.raw = 0xffff;
   vm_state.ss.limit.raw = 0xffff;
   vm_state.ds.limit.raw = 0xffff;
   vm_state.es.limit.raw = 0xffff;
   vm_state.fs.limit.raw = 0xffff;
   vm_state.gs.limit.raw = 0xffff;

   vm_state.cs.attributes.raw = SVM_CODE_16_R0_CO_ATTR;
   vm_state.ss.attributes.raw = SVM_DATA_16_R0_ATTR;
   vm_state.ds.attributes.raw = SVM_DATA_16_R3_ATTR;
   vm_state.es.attributes.raw = SVM_DATA_16_R3_ATTR;
   vm_state.fs.attributes.raw = SVM_DATA_16_R3_ATTR;
   vm_state.gs.attributes.raw = SVM_DATA_16_R3_ATTR;

   rd_msr_amd_efer(vm_state.efer);
   vm_state.efer.eax &= 0xffff;
   vm_state.efer.edx  = 0;
   vm_state.efer.lme  = 0;
   vm_state.efer.lma  = 0;
   vm_state.efer.nxe  = 0;
   vm_state.efer.svme = 1;

   rd_msr_pat(vm_state.g_pat);

   vm_state.cr0.et = 1;

   vm_state.rflags.r1 = 1;
   vm_state.rflags.IF = 1;

   vm_state.kernel_gs_base.low = vm_state.gs.base.low;

   vm_state.rip.low = VM_ENTRY_POINT;
   vm_state.rsp.low = RM_BASE_SP;
}

