/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#include <dev_ata.h>
#include <dev_io_ports.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

/*
** ATA Command/Register access dispatcher
*/
int dev_ata(ata_t __unused__ *ata, io_insn_t *io)
{
   switch(io->port)
   {
   case ATA_STATUS_REG(ATA1_START_PORT):
   {
      if(io->in)
	 debug(DEV_ATA, "ata status\n");
      else
	 debug(DEV_ATA, "ata cmd\n");
      break;
   }
   case ATA_DATA_REG(ATA1_START_PORT):
      debug(DEV_ATA, "ata data\n");
      break;
   case ATA_ERR_REG(ATA1_START_PORT):
   {
      if(io->in)
	 debug(DEV_ATA, "ata err\n");
      else
	 debug(DEV_ATA, "ata feat\n");
      break;
   }
   case ATA_SECT_CNT_REG(ATA1_START_PORT):
      debug(DEV_ATA, "ata sector cnt\n");
      break;
   case ATA_LBA_LOW_REG(ATA1_START_PORT):
      debug(DEV_ATA, "ata lba low\n");
      break;
   case ATA_LBA_MID_REG(ATA1_START_PORT):
      debug(DEV_ATA, "ata lba mid\n");
      break;
   case ATA_LBA_HIGH_REG(ATA1_START_PORT):
      debug(DEV_ATA, "ata lba high\n");
      break;
   case ATA_DRIVE_HEAD_REG(ATA1_START_PORT):
      debug(DEV_ATA, "ata drive head\n");
      break;
   default:
      debug(DEV_ATA, "ata ???\n");
   }

   /* XXX: use direct io instead to speed up */
   return dev_io_proxify(io);
}
