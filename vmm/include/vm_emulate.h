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
#ifndef __VM_EMULATE_H__
#define __VM_EMULATE_H__

#include <vmm.h>

/*
** Functions
*/

/*
** Push/Pop 16 bits values into/from
** 16 bits stack using VM state
**
** Take care about segment
** wrap-around
*/
#define vm_push_16(val)				\
   ({						\
      loc_t stack;				\
      vm_rsp16 -= sizeof(uint16_t);		\
      stack.linear = vm_ss() + vm_rsp16();	\
      *stack.u16 = (val);			\
   })

#define vm_pop_16()				\
   ({						\
      loc_t    stack;				\
      uint16_t val;				\
      stack.linear = vm_ss() + vm_rsp16();	\
      val = *stack.u16;				\
      vm_rsp16 += sizeof(uint16_t);		\
      val;					\
   })


#endif
