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
#ifndef __VMX_EXIT_INT_H__
#define __VMX_EXIT_INT_H__

#include <types.h>

/*
** XXX
*/
#define preempt()                 fixme /*asm volatile( "sti ; nop ; cli" )*/

/*
** Functions
*/
void  __vmx_vmexit_setup_interrupt_window_exiting(uint8_t, uint8_t);
void  __vmx_vmexit_inject_interrupt(uint8_t);


#endif
