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
#ifndef __CPU_H__
#define __CPU_H__

#include <config.h>

/*
** Functions
*/
#ifdef CONFIG_ARCH_AMD
void svm_vm_cpu_skillz_init();
void svm_cpu_max_addr();
#define vm_cpu_skillz_init()      svm_vm_cpu_skillz_init()
#define cpu_max_addr()            svm_cpu_max_addr()
#else
void vmx_vm_cpu_skillz_init();
void vmx_cpu_max_addr();
#define vm_cpu_skillz_init()      vmx_vm_cpu_skillz_init()
#define cpu_max_addr()            vmx_cpu_max_addr()
#endif

void cpu_init();

#endif
