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
#include <stack_trace.h>
#include <info_data.h>
#include <print.h>
#include <asm.h>

extern info_data_t *info;

#define STACK_BASE  ((offset_t*)info->vmm.stack_bottom)

void stack_trace()
{
   offset_t *rbp, *rip;

   printf("\n------ Stack Trace [ esp: 0x%X ]"
	  " vmm stack boundaries [ 0x%X - 0x%X ] ------\n",
	  get_rsp(),
	  page_align(info->vmm.stack_bottom - VMM_MIN_STACK_SIZE),
	  info->vmm.stack_bottom);

   rbp = (offset_t*)get_rbp();

   while(rbp && rbp<STACK_BASE)
   {
      rip = (offset_t*)(*(rbp+1));
      rbp = (offset_t*)(*rbp);
      printf("%X\n", (offset_t)rip - info->vmm.base);
   }
}
