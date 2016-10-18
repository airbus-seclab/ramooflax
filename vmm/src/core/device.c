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
#include <dev.h>
#include <vm.h>
#include <npg.h>
#include <sio.h>
#include <dev_io_ports.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

void dev_a20_set(uint8_t on)
{
   debug(DEV, "setting A20 %d (CR0 0x%x)\n", on, __cr0.raw);

   /* if(!__rmode()) */
   /*    panic("accessing a20 while in protected mode !"); */

   if(on != info->vm.dev.mem.a20)
   {
      info->vm.dev.mem.a20 = on;
      npg_setup_a20();
   }
}

int dev_access()
{
   io_insn_t io = {.raw = 0};
   int       rc = __io_init(&io);

   if(rc != VM_DONE)
      return rc;

#if defined(CONFIG_HAS_NET) || (defined(CONFIG_HAS_EHCI) && defined(CONFIG_EHCI_2ND))
   if(io.port == PCI_CONFIG_ADDR || io.port == PCI_CONFIG_DATA)
      return dev_pci(&io);
#endif

   if(io.port == SIO_INDEX || io.port == SIO_DATA)
      return dev_sio(&io);

   if(io.port == info->hrd.acpi.pm1_ctl_port)
      return dev_acpi_pm1_ctl(&io);

   if(io.port == PS2_SYS_CTRL_PORT_A)
      return dev_ps2(&info->vm.dev.ps2, &io);

   if(range(io.port, KBD_START_PORT, KBD_END_PORT))
      return dev_kbd(&info->vm.dev.kbd, &io);

   if(range(io.port, COM1_START_PORT, COM1_END_PORT))
      return dev_uart(&info->vm.dev.uart, &io);

   if(range(io.port, ATA1_START_PORT, ATA1_END_PORT) || io.port == ATA1_CTRL_PORT)
      return dev_ata(&info->vm.dev.ata[0], &io);

   return dev_io_proxify(&io);
}
