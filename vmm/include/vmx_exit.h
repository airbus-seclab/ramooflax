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
#ifndef __VMX_EXIT_H__
#define __VMX_EXIT_H__

#include <types.h>


#define vmx_vmexit_resolve(x)						\
   ({									\
      int rc;								\
      if( x <= VMCS_VM_EXIT_INFORMATION_BASIC_REASON_MAX )		\
	 rc = vmx_vmexit_resolvers[(x)]();				\
      else								\
	 rc = 0;							\
      rc;								\
   })

/*
** Functions
*/
typedef int (*vmexit_hdlr_t)();

int   vmx_vmexit_resolve_dispatcher();
int   vmx_vmexit_idt_deliver();
int   __vmx_vmexit_idt_deliver_rmode();
int   __vmx_vmexit_idt_deliver_pmode();
int   vmx_vmexit_resolve_vmcall();
void  vmx_release_irq();
void  vmx_vmexit_handler(uint32_t, uint32_t) __attribute__ ((regparm(2)));


#endif
