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
#include <stack_trace.h>
#include <print.h>
#include <asm.h>

extern offset_t __kernel_start__;

void stack_trace()
{
   offset_t *rbp, *rip;

   printf("\n------ Setup Stack Trace [rsp: 0x%X | rip 0x%X]\n",
          get_rsp(), get_pc());

   rbp = (offset_t*)get_rbp();

   while(rbp && rbp < &__kernel_start__)
   {
      rip = (offset_t*)(*(rbp+1));
      rbp = (offset_t*)(*rbp);
      printf("%X\n", (offset_t)rip);
   }
}
