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
#include <svm_vmm.h>
#include <svm_insn.h>
#include <info_data.h>

extern info_data_t *info;

void svm_vmm_init()
{
   wr_msr_host_state(&info->vm.cpu.vmc->vmm_vmcb);
   set_amd_efer((AMD_EFER_SVME|AMD_EFER_NXE), 1);
   svm_vmsave(&info->vm.cpu.vmc->vmm_vmcb);
}
