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
#include <ctrl.h>
#include <gdb.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;
static uint8_t __ctrl_input[1024];

void ctrl_logic()
{
   uint8_t *ptr;
   size_t  len, remain = 0;

   do
   {
      do
      {
	 len = remain;
	 remain = 0;
	 ptr = &__ctrl_input[len];

	 len += ctrl_read(ptr, sizeof(__ctrl_input) - len);

	 if(len)
	 {
	    remain = gdb_stub(__ctrl_input, len);
	    if(remain && remain != len)
	    {
	       ptr = __ctrl_input + (len - remain);
	       memcpy(__ctrl_input, ptr, remain);
	    }
	 }

      } while(remain);

   } while(ctrl_active());
}

void ctrl_post()
{
   gdb_stub_post();
}

void vmm_ctrl()
{
   if(gdb_enabled() || !(info->vmm.ctrl.vmexit_cnt.raw % CTRL_RATIO))
      ctrl_logic();

   ctrl_post();
}
