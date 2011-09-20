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
#include <io.h>
#include <dev_io_ports.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

static int __io_insn_string_in_fwd(io_insn_t *io, io_size_t *sz)
{
   if(!vm_write_mem(io->dst.linear, io->src.u8, sz->miss))
   {
      debug(DEV_IO, "io fwd ins: vm_write_mem() fail\n");
      return 0;
   }

   sz->done      += sz->miss;
   sz->available -= sz->miss;
   sz->miss       = 0;
   return 1;
}

static int __io_insn_string_out_fwd(io_insn_t *io, io_size_t *sz)
{
   if(!vm_read_mem(io->src.linear, io->dst.u8, sz->miss))
   {
      debug(DEV_IO, "io fwd outs: vm_read_mem() fail\n");
      return 0;
   }

   sz->done      += sz->miss;
   sz->available -= sz->miss;
   sz->miss       = 0;
   return 1;
}

static int __io_insn_string_in_bwd(io_insn_t __unused__ *io, io_size_t __unused__ *sz)
{
   debug(DEV_IO, "io bwd ins: not implemented !\n");
   return 0;
}

static int __io_insn_string_out_bwd(io_insn_t __unused__ *io, io_size_t __unused__ *sz)
{
   debug(DEV_IO, "io bwd outs: not implemented !\n");
   return 0;
}

static int __io_insn_string_in(io_insn_t *io, io_size_t *sz)
{
   uint64_t update = info->vm.cpu.gpr->rdi.raw & io->msk;

   if(io->back)
   {
      if(!__io_insn_string_in_bwd(io, sz))
	 return 0;

      update -= (sz->done & io->msk);
   }
   else
   {
      if(!__io_insn_string_in_fwd(io, sz))
	 return 0;

      update += (sz->done & io->msk);
   }

   info->vm.cpu.gpr->rdi.raw &= ~io->msk;
   info->vm.cpu.gpr->rdi.raw |= update;
   return 1;
}

static int __io_insn_string_out(io_insn_t *io, io_size_t *sz)
{
   uint64_t update = info->vm.cpu.gpr->rsi.raw & io->msk;

   if(io->back)
   {
      if(!__io_insn_string_out_bwd(io, sz))
	 return 0;

      update -= (sz->done & io->msk);
   }
   else
   {
      if(!__io_insn_string_out_fwd(io, sz))
	 return 0;

      update += (sz->done & io->msk);
   }

   info->vm.cpu.gpr->rsi.raw &= ~io->msk;
   info->vm.cpu.gpr->rsi.raw |= update;
   return 1;
}

/*
** XXX: need security checks on linear address (segments, ...)
*/
static int __io_insn_string(io_insn_t *io, void *device, io_size_t *sz)
{
   if(io->sz != 1)
   {
      debug(DEV_IO, "string io size != 1 not supported\n");
      return 0;
   }

   if(io->rep)
      sz->miss *= (info->vm.cpu.gpr->rcx.raw & io->msk);

   if(io->in)
   {
      io->src.addr = device;
      __string_io_linear(io->dst.linear, io);

      if(!__io_insn_string_in(io, sz))
	 return 0;
   }
   else
   {
      io->dst.addr = device;
      __string_io_linear(io->src.linear, io);

      if(!__io_insn_string_out(io, sz))
	 return 0;
   }

   if(io->rep)
   {
      info->vm.cpu.gpr->rcx.raw &= ~io->msk;
      info->vm.cpu.gpr->rcx.raw |=
	 (info->vm.cpu.gpr->rcx.raw & io->msk) - ((sz->done/io->sz) & io->msk);
   }

   if(sz->miss)
   {
      debug(DEV_IO, "missing %d bytes, available %d, done %d !\n",
	    sz->miss, sz->available, sz->done);
      return 0;
   }

   return 1;
}

static int __io_insn_simple(io_insn_t *io, void *device, io_size_t *sz)
{
   if(io->in)
   {
      io->dst.addr = (void*)&info->vm.cpu.gpr->rax.low;
      io->src.addr = device;
   }
   else
   {
      io->dst.addr = device;
      io->src.addr = (void*)&info->vm.cpu.gpr->rax.low;
   }

   if(io->sz != 1)
   {
      debug(DEV_IO, "simple io size != 1 not supported\n");
      return 0;
   }

   *io->dst.u8 = *io->src.u8;

   sz->done      += io->sz;
   sz->available -= io->sz;
   sz->miss      -= io->sz;

   return 1;
}

static void dev_io_pic_proxy(io_insn_t *io, uint8_t data)
{
   static uint8_t wait_icw2 = 0;

   if(!range(io->port, PIC1_START_PORT, PIC1_END_PORT) || io->in)
      return;

   if((io->port & 1) == 0)
   {
      if(is_pic_icw1(data))
	 wait_icw2 = 1;
   }
   else if(wait_icw2)
   {
      wait_icw2 = 0;
      info->vm.dev.pic1_icw2 = data;
      __allow_io_range(PIC1_START_PORT, PIC2_START_PORT);
      debug(DEV_IO, "detected PIC1 icw2 setup 0x%x -- release intercept\n", data);
   }
}

static int dev_io_string_proxify(io_insn_t *io)
{
   io_size_t sz;
   raw32_t   data;

   sz.done = 0;
   sz.miss = io->sz;
   sz.available = sizeof(raw32_t);

   if(io->rep)
   {
      debug(DEV_IO, "rep prefix not implemented on proxy string io\n");
      return 0;
   }

   if(io->sz != 1)
   {
      debug(DEV_IO, "proxy string io size != 1 not supported\n");
      return 0;
   }

   /* backward not checked as we only allow sz = 1 and no rep */

   if(io->in)
   {
      insb(&data, io->port);
      return __io_insn_string(io, (void*)&data, &sz);
   }

   if(!__io_insn_string(io, (void*)&data, &sz))
      return 0;

   dev_io_pic_proxy(io, data.blow);
   outsb(&data, io->port);
   return 1;
}

static int dev_io_simple_proxify(io_insn_t *io)
{
   io_size_t sz;
   raw32_t   data;

   sz.done = 0;
   sz.miss = io->sz;
   sz.available = sizeof(raw32_t);

   if(io->in)
   {
      switch(io->sz)
      {
      case 1: data.blow = inb(io->port); break;
      case 2: data.wlow = inw(io->port); break;
      case 4: data.raw  = inl(io->port); break;
      }

      debug(DEV_IO, "proxy io in 0x%x data 0x%x\n", io->port, data.raw);
      return __io_insn_simple(io, &data, &sz);
   }
   else
   {
      if(!__io_insn_simple(io, &data, &sz))
	 return 0;

      debug(DEV_IO, "proxy io out 0x%x data 0x%x\n", io->port, data.raw);

      switch(io->sz)
      {
      case 1:
	 dev_io_pic_proxy(io, data.blow);
	 outb(data.blow, io->port);
	 break;

      case 2: outw(data.wlow, io->port); break;
      case 4: outl(data.raw,  io->port); break;
      }
      return 1;
   }
}

int dev_io_insn(io_insn_t *io, void *device, io_size_t *sz)
{
   sz->done = 0;
   sz->miss = io->sz;

   if(io->s)
      return __io_insn_string(io, device, sz);

   return __io_insn_simple(io, device, sz);
}

int dev_io_proxify(io_insn_t *io)
{
   if(io->s)
      return dev_io_string_proxify(io);

   return dev_io_simple_proxify(io);
}
