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
#include <mp.h>
#include <dev_kbd.h>
#include <dev_pic.h>
#include <dev_uart.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void vmm_dev_init()
{
   dev_kbd_init(&info->vm.dev.kbd);
   dev_uart_init(&info->vm.dev.uart, SERIAL_COM1);
}

static void vmm_cpu_init()
{
   info->vm.cpu.dflt_excp = VM_RMODE_EXCP_BITMAP;
   vmm_cpu_init_arch();
}

void __attribute__((regparm(1))) vmm_start()
{
   vm_set_entry();
   vmm_cpu_start_arch();
}

void vmm_init()
{
   vmm_cpu_init();
   vmm_dev_init();
}
