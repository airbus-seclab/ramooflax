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
#include <sio.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

/* Super I/O chip
**
** OSPM disables devices upon S3
** we want to get our serial port back
**
** Super I/O is controlled by index/data register
** COM1: bit 2 from index 0
**
** Definition of the I/O port has been retrieved
** by dumping acpi DSDT and disasm AML code
** - reading _DIS method of COMA object
** - reading definition of variable FCMA variable
** - finding SIO region/indexField definition
**   that covers FCMA variable
*/
static int __dev_sio_filter_com1(void *data, void __unused__ *arg)
{
   uint8_t *com1 = (uint8_t*)data;

   if(! (*com1 & 2))
   {
      debug(DEV_SIO, "Super IO disable COM1 (prevent)\n");
      *com1 |= 2;
   }

   return VM_DONE;
}

static int __dev_sio_filter_index(void *data, void __unused__ *arg)
{
   uint8_t index = *(uint8_t*)data;

   debug(DEV_SIO, "Super IO access index %d\n", index);

   if(!index)
   {
      __deny_io(SIO_DATA);
      info->vm.dev.sio_filter = __dev_sio_filter_com1;
   }
   else
      __allow_io(SIO_DATA);

   return VM_DONE;
}

int dev_sio(io_insn_t *io)
{
   switch(io->port)
   {
   case SIO_INDEX: info->vm.dev.sio_filter = __dev_sio_filter_index; break;
   case SIO_DATA: /* set by sio filter index */ break;
   default: info->vm.dev.sio_filter = NULL;
   }

   return dev_io_proxify_filter(io, info->vm.dev.sio_filter, NULL);
}
