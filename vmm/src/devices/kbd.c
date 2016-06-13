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
#include <dev_kbd.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

int __dev_kbd_ctrl(kbd_t *kbd, io_insn_t *io)
{
   if(io->in)
      info->vm.cpu.gpr->rax.blow = in(io->port);
   else
   {
      kbd->last_cmd = info->vm.cpu.gpr->rax.blow;
      out(info->vm.cpu.gpr->rax.blow, io->port);
   }

   return VM_DONE;
}

int __dev_kbd_data(kbd_t *kbd, io_insn_t *io)
{
   raw64_t *rax = &info->vm.cpu.gpr->rax;

   if(io->in)
   {
      rax->blow = in(io->port);

      /* Return the value the VM expects to see for the a20 bit */
      if(kbd->last_cmd == KBD_CMD_READ_O_PORT)
      {
         rax->blow &= ~KBD_OUTPUT_PORT_A20;
         rax->blow |= (kbd->output_port.a20 ? KBD_OUTPUT_PORT_A20 : 0);
      }
   }
   else
   {
      if(kbd->last_cmd == KBD_CMD_WRITE_O_PORT)
      {
         kbd->output_port.raw = rax->blow;

         /* if(kbd->output_port.sys_rst) */
         /*    panic("keyboard controller system reset"); */

         dev_a20_set(kbd->output_port.a20);

         /* Don't let the VM _actually_ disable the a20 line */
         if(!kbd->output_port.a20)
            rax->blow |= KBD_OUTPUT_PORT_A20;
      }

      out(rax->blow, io->port);
   }

   return VM_DONE;
}

int dev_kbd(kbd_t *kbd, io_insn_t *io)
{
   switch(io->port)
   {
   case KBD_CTRL_PORT:
      return __dev_kbd_ctrl(kbd, io);
   case KBD_DATA_PORT:
      return __dev_kbd_data(kbd, io);
   }

   if(io->in)
      info->vm.cpu.gpr->rax.blow = in(io->port);
   else
      out(info->vm.cpu.gpr->rax.blow, io->port);

   return VM_DONE;
}
