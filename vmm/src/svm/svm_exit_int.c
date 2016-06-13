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
#include <svm_exit_int.h>
#include <intr.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

/* void __svm_vmexit_setup_interrupt_window_exiting(uint8_t enable, uint8_t irq) */
/* { */
/*    if(enable ^ vm_ctrls.sys_insn_bitmap.vintr) */
/*    { */
/*       vm_ctrls.sys_insn_bitmap.vintr = enable?1:0; */
/*       vm_ctrls.int_ctrl.v_irq = enable?1:0; */

/*       if(enable) */
/*       vm_ctrls.int_ctrl.v_intr_vector = irq; */
/*    } */
/* } */

/* void __svm_vmexit_inject_virtual_interrupt(uint8_t vector) */
/* { */
/*    uint8_t prio; */

/*    if(vector < 16) */
/*       prio = 15; */
/*    else */
/*       prio = vector/16; */

/*    vm_ctrls.int_ctrl.v_intr_vector = vector; */
/*    vm_ctrls.int_ctrl.v_intr_prio = prio; */
/*    vm_ctrls.int_ctrl.v_irq = 1; */
/* } */
