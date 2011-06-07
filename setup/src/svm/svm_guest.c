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
#include <svm_guest.h>
#include <svm_vm.h>
#include <msr.h>
#include <info_data.h>

extern info_data_t *info;

void svm_vmcb_guest_state_init()
{
   vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area;

   //state->idtr.limit.wlow = BIOS_MISC_INTERRUPT*sizeof(ivt_e_t) - 1;
   state->idtr.limit.wlow = 0x3ff;

   state->ss.selector.raw = RM_BASE_SS;
   state->ss.base_addr.low = (state->ss.selector.raw)<<4;

   state->cs.limit.raw = 0xffff;
   state->ss.limit.raw = 0xffff;
   state->ds.limit.raw = 0xffff;
   state->es.limit.raw = 0xffff;
   state->fs.limit.raw = 0xffff;
   state->gs.limit.raw = 0xffff;

   state->cs.attributes.raw = SVM_CODE_16_R0_CO_ATTR;
   state->ss.attributes.raw = SVM_DATA_16_R0_ATTR;
   state->ds.attributes.raw = SVM_DATA_16_R3_ATTR;
   state->es.attributes.raw = SVM_DATA_16_R3_ATTR;
   state->fs.attributes.raw = SVM_DATA_16_R3_ATTR;
   state->gs.attributes.raw = SVM_DATA_16_R3_ATTR;

   rd_msr_amd_efer(state->efer);
   state->efer.eax &= 0xffff;
   state->efer.edx  = 0;
   state->efer.lme  = 0;
   state->efer.lma  = 0;
   state->efer.nxe  = 0;
   state->efer.svme = 1;

   rd_msr_pat(state->g_pat);

   state->cr0.et = 1;

   state->rflags.r1 = 1;
   state->rflags.IF = 1;

   state->kernel_gs_base.low = state->gs.base_addr.low;

   state->rip.low = RM_BASE_IP;
   state->rsp.low = RM_BASE_SP;
}

