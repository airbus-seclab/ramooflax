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
#include <gdbstub.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int gdb_opt_hdl(char *str, void *arg)
{
   uint64_t *data = (uint64_t*)arg;
   return dec_to_uint64((uint8_t*)str, strlen(str), data);
}

static int gdb_params(gdbstub_t *gdb, mbi_t *mbi)
{
   module_t      *mod = (module_t*)((offset_t)mbi->mods_addr + sizeof(module_t));
   mbi_opt_hdl_t  hdl = (mbi_opt_hdl_t)gdb_opt_hdl;

   if(!mbi_get_opt(mbi, mod, "gdb_rate", hdl, (void*)&gdb->rate))
      gdb->rate = GDB_DFT_RATE;

   return VM_DONE;
}

void gdb_init(mbi_t *mbi)
{
   debug(GDBSTUB, "\n- gdbstub init\n");

   if(gdb_params(&info->vmm.gstub, mbi) != VM_DONE)
      panic("failed to init gdbstub");
}
