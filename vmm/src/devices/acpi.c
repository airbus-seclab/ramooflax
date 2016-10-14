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
#include <acpi.h>
#include <ctrl_evt.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/* static int acpi_pm1_sts(io_insn_t *io) */
/* { */
/*    int rc; */
/*    static uint16_t pm1_sts; */
/*    io_size_t sz; */

/*    sz.available = sizeof(pm1_sts); */

/*    if(io->in) */
/*    { */
/*       if((rc=dev_io_native(io, &pm1_sts)) != VM_DONE) */
/*          return rc; */

/*       debug(VM, "read PM1 status 0x%x\n", pm1_sts); */

/*       if(!(pm1_sts & 1<<9)) */
/*       { */
/*          debug(VM, "faking SLP button\n"); */
/*          pm1_sts |= 1<<9; */
/*       } */

/*       return dev_io_insn(io, &pm1_sts, &sz); */
/*    } */

/*    if((rc=dev_io_insn(io, &pm1_sts, &sz)) != VM_DONE) */
/*       return rc; */

/*    debug(VM, "write PM1 status 0x%x\n", pm1_sts); */

/*    return dev_io_native(io, &pm1_sts); */
/* } */

int dev_acpi_pm1_ctl(io_insn_t *io)
{
   acpi_pm1_ctl_t reg;
   int            rc;
   io_size_t      sz;

   sz.available = sizeof(reg);

   if(io->in)
   {
      if((rc=dev_io_native(io, &reg)) != VM_DONE)
         return rc;

      debug(VM, "read acpi pm1 control 0x%x\n", reg.raw);
      return dev_io_insn(io, &reg, &sz);
   }

   if((rc=dev_io_insn(io, &reg, &sz)) != VM_DONE)
      return rc;

   debug(VM, "write acpi pm1 control 0x%x\n", reg.raw);

   if(reg.slp_en && reg.slp_tp == info->hrd.acpi.s3)
   {
      debug(VM, "acpi S3 mode enabled\n");
      ctrl_evt_suspend();
      return acpi_wake_up();
   }

   return dev_io_native(io, &reg);
}
