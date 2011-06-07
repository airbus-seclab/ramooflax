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
#include <vmm.h>
#include <amd.h>
#include <pci.h>
#include <msr.h>
#include <svm_vmm.h>
#include <svm_vm.h>
#include <svm_insn.h>
#include <print.h>
#include <info_data.h>

extern info_data_t *info;

/* static void svm_mask_cores() */
/* { */
/*    pci_cfg_addr_t               pci; */
/*    amd_conf_addr_htt_node_id_t  node; */
/*    amd_conf_addr_htt_ctrl_reg_t ctrl; */

/*    /\* HTT function *\/ */
/*    pci.bus  = 0; */
/*    pci.dev  = 24; */
/*    pci.fnc  = AMD_CONFIG_ADDR_HTT_FUNC; */
/*    pci.enbl = 1; */

/*    /\* node register *\/ */
/*    pci.reg = AMD_CONFIG_ADDR_HTT_NODE_REG; */

/*    node.raw = pci_cfg_read(pci); */
/*    node.node_nr = 0; */
/*    node.cpu_cnt = 1; */
/*    pci_cfg_write(pci, node.raw); */

/*    /\* ctrl register *\/ */
/*    pci.reg = AMD_CONFIG_ADDR_HTT_CTRL_REG; */

/*    ctrl.raw = pci_cfg_read(pci); */
/*    ctrl.cpu1_en = 0; */
/*    pci_cfg_write(pci, ctrl.raw); */
/* } */

static void svm_vmm_cpu_init()
{
   amd_vmcr_msr_t vmcr;

   if(!svm_supported())
      panic("svm not supported");

   if(!svm_npt_supported())
      panic("svm npt feature not supported");

   rd_msr_vmcr(vmcr);
   if(vmcr.svm_dis && vmcr.lock)
      panic("svm feature BIOS-locked");

   wr_msr_host_state(&info->vm.cpu.vmc->vmm_vmcb);
   svm_enable();

   svm_vmsave(&info->vm.cpu.vmc->vmm_vmcb);
   svm_vm_init();

   /* vmrun needs vmcb addr in eax */
   info->vm.cpu.gpr->rax.raw = (offset_t)&info->vm.cpu.vmc->vm_vmcb;
}

void svm_vmm_init()
{
   //svm_mask_cores();
   svm_vmm_cpu_init();
}
