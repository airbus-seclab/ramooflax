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
#include <dev_kbd.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

int __dev_kbd_ctrl(kbd_t *kbd, io_insn_t *io)
{
   if(io->d)
      info->vm.cpu.gpr->rax.blow = in(io->port);
   else
   {
      kbd->last_cmd = info->vm.cpu.gpr->rax.blow;
      out(info->vm.cpu.gpr->rax.blow, io->port);
   }

   return 1;
}

int __dev_kbd_data(kbd_t *kbd, io_insn_t *io)
{
   if(io->d)
   {
      info->vm.cpu.gpr->rax.blow = in(io->port);

      if(kbd->last_cmd == KBD_CMD_READ_O_PORT && kbd->output_port.a20)
	 info->vm.cpu.gpr->rax.blow |= KBD_OUTPUT_PORT_A20;
   }
   else
   {
      uint8_t al = info->vm.cpu.gpr->rax.blow;

      if(kbd->last_cmd == KBD_CMD_WRITE_O_PORT)
      {
	 kbd->output_port.raw = al;
	 dev_a20_set(kbd->output_port.a20);

	 if(kbd->output_port.a20)
	    al &= ~KBD_OUTPUT_PORT_A20;
      }

      out(al, io->port);
   }

   return 1;
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

   if(io->d)
      info->vm.cpu.gpr->rax.blow = in(io->port);
   else
      out(info->vm.cpu.gpr->rax.blow, io->port);

   return 1;
}
