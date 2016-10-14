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
#include <vm.h>
#include <dev_kbd.h>
#include <dev_pic.h>
#include <dev_uart.h>
#include <dev_io_ports.h>
#include <sio.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void vm_dev_init()
{
   info->vm.dev.mem.a20 = 1;

   dev_kbd_init(&info->vm.dev.kbd);
   dev_uart_init(&info->vm.dev.uart, SERIAL_COM1);
   dev_ata_init(&info->vm.dev.ata[0], ATA1_START_PORT);

   /* proxify to detect rebase for uart irq injection */
   info->vm.dev.pic1_icw2 = DFLT_PIC1_ICW2;
   __deny_io_range(PIC1_START_PORT, PIC1_END_PORT);

   /* lazzy emulation */
   __deny_io_range(COM1_START_PORT, COM1_END_PORT);

   /* monitor reboot and A20 */
   __deny_io_range(KBD_START_PORT, KBD_END_PORT);
   __deny_io(PS2_SYS_CTRL_PORT_A);

   /* ata filtering */
   /* __deny_io(ATA1_CTRL_PORT); */
   /* __deny_io_range(ATA1_START_PORT, ATA1_END_PORT); */

   /* prevent PCI device detection */
#if defined(CONFIG_HAS_NET) || \
   (defined(CONFIG_HAS_EHCI) && defined(CONFIG_EHCI_2ND))
   __deny_io(PCI_CONFIG_ADDR);
#endif

#if defined(CONFIG_HAS_NET) && defined(CONFIG_HAS_E1000)
   {
      e1k_info_t *e1k = &info->hrd.dev.net.arch;
      npg_unmap(e1k->base.linear, e1k->base.linear + (128<<10));
      debug(E1000, "protect e1000 mmio space [0x%X - 0x%X]\n"
            ,e1k->base.linear, e1k->base.linear + (128<<10));
   }
#endif

   /* super io */
   __deny_io(SIO_INDEX);
}

static void vm_cpu_init()
{
   info->vm.cpu.dflt_excp = VM_RMODE_EXCP_BITMAP;
}

void vm_init()
{
   vm_cpu_init();
   vm_dev_init();
   vm_vmc_init();
   vm_set_entry();
}
