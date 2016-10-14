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
void sio_check_com1()
{
   uint8_t com;

   outb(0, SIO_INDEX);
   com = inb(SIO_DATA);

   if(! (com & 2))
      outb(com|2, SIO_DATA);
}
