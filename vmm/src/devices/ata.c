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
   ata_dev_reg_t dev;
   io_size_t     sz = {.available = sizeof(ata_dev_reg_t)};

   if(io->in)
      return dev_io_proxify(io);
   else
   {
      /* check crazy io (should not happen) */
      if((io->sz * io->cnt) > sizeof(ata_dev_reg_t))
      {
	 debug(DEV_ATA, "unsupported ata dev access\n");
	 return 0;
      }

      if(!dev_io_insn(io, (void*)&dev.raw, &sz))
   	 return 0;

      ata->dev_head = dev;
      debug(DEV_ATA, "ata device [%s]\n", dev.dev ? "SLAVE":"MASTER");
      return dev_io_native(io, &dev.raw);
   }
}

int __fake_ata_status(ata_t *ata)
{
   if(ata->last_out == ATA_CMD_REG(ata->base))
      info->vm.cpu.gpr->rax.blow = 1;
   else if(ata->last_out == ATA_DEVICE_REG(ata->base))
      info->vm.cpu.gpr->rax.blow = 0;
   else
   {
      debug(DEV_ATA, "can't fake status for previous out(0x%x)\n", ata->last_out);
      return 0;
   }

   return 1;
}

int __dev_ata_status(ata_t *ata, io_insn_t *io)
{
   if(!__rmode() && __ata_guest_want_slave(ata))
   {
      debug(DEV_ATA, "ata fake status\n");
      return __fake_ata_status(ata);
   }

   debug(DEV_ATA, "ata status\n");
   return dev_io_proxify(io);
}

int __dev_ata_alt_status(ata_t *ata, io_insn_t *io)
{
   if(!__rmode() && __ata_guest_want_slave(ata))
   {
      debug(DEV_ATA, "ata fake ALT status\n");
      return __fake_ata_status(ata);
   }

   debug(DEV_ATA, "ata ALT status\n");
   return dev_io_proxify(io);
}

int __dev_ata_lba_filter(void *device, void *arg)
{
   ata_t     *ata  = (ata_t*)arg;
   ata_dev_t *disk = &ata->devices[0];
   uint8_t    lba  = *(uint8_t*)device;
   uint8_t    idx  = ata->last_out - ATA_LBA_LOW_REG(ata->base);

   if(idx > 2)
   {
      debug(DEV_ATA, "unknown (internal) LBA index access (%d)\n", idx);
      return 0;
   }

   disk->lba[idx] = lba;
   debug(DEV_ATA, "ata lba[%d] = 0x%x\n", idx, lba);
   return 1;
}

int __dev_ata_scnt_filter(void *device, void *arg)
{
   ata_t     *ata = (ata_t*)arg;
   ata_dev_t *disk = &ata->devices[0];

   disk->cnt = *(uint8_t*)device;
   debug(DEV_ATA, "ata sector cnt [0x%x]\n", disk->cnt);
   return 1;
}

int __dev_ata_cmd_filter(void *device, void *arg)
{
   ata_t *ata = (ata_t*)arg;

   ata->cmd = *(uint8_t*)device;

   if(ata->cmd == ATA_READ_SECTOR_CMD || ata->cmd == ATA_WRITE_SECTOR_CMD)
   {
      ata_dev_t *disk = &ata->devices[0];
      uint32_t   lba  = __ata_build_lba(ata, disk);

      debug(DEV_ATA, "ata cmd [%s 0x%x sector(s) from 0x%x]\n"
	    ,(ata->cmd == ATA_WRITE_SECTOR_CMD) ? "write":"read"
	    ,disk->cnt, lba);
   }
   else
   {
      char *str;
      switch(ata->cmd)
      {
      case ATA_IDENT_DEV_CMD      : str = "IDENT_DEV"; break;
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
   }

   return 1;
}

/*
** ATA Command/Register access dispatcher
*/
int dev_ata(ata_t *ata, io_insn_t *io)
{
   if(!io->in)
      ata->last_out = io->port;

   if(io->port == ATA_STATUS_REG(ata->base))
   {
      if(io->in)
	 return __dev_ata_status(ata, io);
      else
	 return dev_io_proxify_filter(io, __dev_ata_cmd_filter, ata);
   }

   if(io->port == ATA_ALT_STATUS_REG(ata->base))
   {
      if(io->in)
	 return __dev_ata_alt_status(ata, io);
      else
	 goto __proxify;
   }

   if(io->port == ATA_DEVICE_REG(ata->base))
      return __dev_ata_device(ata, io);

   if(io->port == ATA_DATA_REG(ata->base))
   {
      debug(DEV_ATA, "ata data\n");
      goto __proxify;
   }

   if(io->port == ATA_SECT_CNT_REG(ata->base))
      return dev_io_proxify_filter(io, __dev_ata_scnt_filter, ata);

   if(io->port == ATA_LBA_LOW_REG(ata->base) ||
      io->port == ATA_LBA_MID_REG(ata->base) ||
      io->port == ATA_LBA_HIGH_REG(ata->base))
      return dev_io_proxify_filter(io, __dev_ata_lba_filter, ata);

   if(io->port == ATA_ERR_REG(ata->base))
   {
      if(io->in)
	 debug(DEV_ATA, "ata err\n");
      else
	 debug(DEV_ATA, "ata feat\n");

      goto __proxify;
   }

   debug(DEV_ATA, "ata ???\n");

__proxify:
   return dev_io_proxify(io);
}
