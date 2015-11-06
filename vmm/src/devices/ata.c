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

int __dev_ata_device(ata_t *ata, io_insn_t *io)
{
   /* if(!io->in) */
   /* { */
   /*    debug(DEV_ATA, "ata device [%s]\n" */
   /* 	    ,(info->vm.cpu.gpr->rax.low & (1<<4))?"SLAVE":"MASTER" ); */

   /*    /\* XXX: should check if kernel is not trying to */
   /*    ** do a string io or simple io with size > 1byte */
   /*    ** we assume here it is a simple outb */
   /*    *\/ */
   /*    ata_dev_reg_t dev; */
   /*    io_size_t     sz = {.available = sizeof(ata_dev_reg_t)}; */

   /*    if(!dev_io_insn(io, (void*)&dev.raw, &sz)) */
   /* 	 return 0; */

   /*    ata->dev_reg = dev; */

   /*    /\* Guest check slave *\/ */
   /*    if(dev.dev && !__rmode()) */
   /* 	 return 1; */

   /*    return dev_io_native(io, &dev.raw); */
   /* } */


   debug(DEV_ATA, "ata device [%s]\n"
	 ,(info->vm.cpu.gpr->rax.low & (1<<4))?"SLAVE":"MASTER" );

   return dev_io_proxify(io);
}

int __dev_ata_status(ata_t *ata, io_insn_t *io)
{
   debug(DEV_ATA, "ata status\n");

   /* last device configured is slave */
   /* if(ata->dev_reg.dev) */
   /* { */
   /*    if(!__rmode()) */
   /*    { */
   /* 	 info->vm.cpu.gpr->rax.blow = 0x0; */
   /* 	 return 1; */
   /*    } */
   /* } */

   return dev_io_proxify(io);
}

int __dev_ata_alt_status(ata_t *ata, io_insn_t *io)
{
   debug(DEV_ATA, "ata ALT status\n");
   return dev_io_proxify(io);
}

int __dev_ata_lba(ata_t *ata, io_insn_t *io, int idx)
{
   char *str;

   switch(idx)
   {
   case 0: str = "low"; break;
   case 1: str = "mid"; break;
   case 2: str = "hig"; break;
   }

   debug(DEV_ATA, "ata lba %s\n", str);
   return dev_io_proxify(io);
}

int __dev_ata_cmd(ata_t *ata, io_insn_t *io)
{
   char *str;

   switch(info->vm.cpu.gpr->rax.blow)
   {
   case ATA_READ_SECTOR_CMD    : str = "READ_SECTOR";break;
   case ATA_WRITE_SECTOR_CMD   : str = "WRITE_SECTOR";break;
   case ATA_IDENT_DEV_CMD      : str = "IDENT_DEV";break;
   case ATA_INIT_DEV_PARAMS_CMD: str = "INIT_DEV_PARAMS";break;
   case ATA_RECALIBRATE_CMD    : str = "RECALIBRATE";break;
   case ATA_DEVICE_RESET_CMD   : str = "DEVICE_RESET";break;
   case ATA_READ_DMA_CMD       : str = "READ_DMA";break;
   case ATA_WRITE_DMA_CMD      : str = "WRITE_DMA";break;
   case ATA_SET_FEAT_CMD       : str = "SET_FEAT";break;
   case ATA_IDENT_PKT_DEV_CMD  : str = "IDENT_PKT_DEV";break;
   case ATA_SET_MULTI_MODE_CMD : str = "SET_MULTI_MODE";break;
   default                     : str = "????";
   }

   debug(DEV_ATA, "ata cmd [%s]\n", str);
   return dev_io_proxify(io);
}

int __dev_ata_scnt(ata_t *ata, io_insn_t *io)
{
   debug(DEV_ATA, "ata sector cnt\n");
   return dev_io_proxify(io);
}

/*
** ATA Command/Register access dispatcher
*/
int dev_ata(ata_t *ata, io_insn_t *io)
{
   if(io->port == ATA_STATUS_REG(ata->base))
   {
      if(io->in)
	 return __dev_ata_status(ata, io);
      else
	 return __dev_ata_cmd(ata, io);
   }

   if(io->port == ATA_DEVICE_REG(ata->base))
      return __dev_ata_device(ata, io);

   if(io->port == ATA_DATA_REG(ata->base))
   {
      debug(DEV_ATA, "ata data\n");
      goto __proxify;
   }

   if(io->port == ATA_SECT_CNT_REG(ata->base))
      return __dev_ata_scnt(ata, io);

   if(io->port == ATA_LBA_LOW_REG(ata->base) ||
      io->port == ATA_LBA_MID_REG(ata->base) ||
      io->port == ATA_LBA_HIGH_REG(ata->base))
      return __dev_ata_lba(ata, io, io->port - ATA_LBA_LOW_REG(ata->base));

   if(io->port == ATA_ERR_REG(ata->base))
   {
      if(io->in)
	 debug(DEV_ATA, "ata err\n");
      else
	 debug(DEV_ATA, "ata feat\n");

      goto __proxify;
   }

   if(io->port == ATA_ALT_STATUS_REG(ata->base))
   {
      if(io->in)
	 return __dev_ata_alt_status(ata, io);
      else
	 goto __proxify;
   }

   debug(DEV_ATA, "ata ???\n");

__proxify:
   return dev_io_proxify(io);
}
