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
#include <dev_ps2.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

int dev_ps2(ps2_t *ps2, io_insn_t *io)
{
   io_size_t sz = { .available = 1 };
   int       rc = dev_io_insn(io, &ps2->raw, &sz);

   if(rc != VM_DONE)
      return rc;

   if(ps2->fast_reset)
      panic("ps2 fast reset !");

   dev_a20_set((uint8_t)ps2->enabled_a20);
   return VM_DONE;
}
