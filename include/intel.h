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
#ifndef __INTEL_CPU_H__
#define __INTEL_CPU_H__

#include <cr.h>

#define X86_OPCODE_VMCALL         0xc1010f

/*
** We enabling interrupts we assume
** that they are off
** We disabling interrupts we assume
** that they are on
*/
#define force_interrupts_on()     asm volatile( "sti ; nop" )
#define force_interrupts_off()    asm volatile( "cli" )
#define halt()                    asm volatile( "sti ; hlt ; cli" )
#define enable_vmx()              ({ulong_t cr4 = get_cr4(); set_cr4(cr4|0x2000UL);})

#endif
