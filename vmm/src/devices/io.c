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
#include <emulate.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

static int __io_insn_string_in_fwd(io_insn_t *io, io_size_t *sz)
{
   if(vm_write_mem(io->dst.linear, io->src.u8, sz->miss) != VM_DONE)
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
   if(vm_read_mem(io->src.linear, io->dst.u8, sz->miss) != VM_DONE)
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

static int __io_insn_string(io_insn_t *io, void *device, io_size_t *sz)
{
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
      info->vm.cpu.gpr->rcx.raw =
	 (info->vm.cpu.gpr->rcx.raw & ~io->msk) | ((sz->miss/io->sz) & io->msk);
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

   switch(io->sz)
   {
   case 1: *io->dst.u8  = *io->src.u8;  break;
   case 2: *io->dst.u16 = *io->src.u16; break;
   case 4: *io->dst.u32 = *io->src.u32; break;
   }

   sz->done      += io->sz;
   sz->available -= io->sz;
   sz->miss      -= io->sz;

   return 1;
}

static void __io_pic_detect(io_insn_t *io, uint8_t data)
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

static int __io_string_native(io_insn_t *io, void *device)
{
   raw32_t *data = (raw32_t*)device;

   if(io->back)
   {
      debug(DEV_IO, "native io bwd string: not implemented !\n");
      return 0;
   }

   debug(DEV_IO, "native io string: %D bytes (cnt %D, sz %d)\n"
	 ,io->cnt*io->sz, io->cnt, io->sz);

   if(io->in)
      switch(io->sz)
      {
      case 1: rep_insb(data, io->port, io->cnt); break;
      case 2: rep_insw(data, io->port, io->cnt); break;
      case 4: rep_insl(data, io->port, io->cnt); break;
      }
   else
      switch(io->sz)
      {
      case 1: __io_pic_detect(io, data->blow);
	      rep_outsb(data, io->port, io->cnt); break;
      case 2: rep_outsw(data, io->port, io->cnt); break;
      case 4: rep_outsl(data, io->port, io->cnt); break;
      }

   return 1;
}

static int __io_simple_native(io_insn_t *io, void *device)
{
   raw32_t *data = (raw32_t*)device;

   if(io->in)
      switch(io->sz)
      {
      case 1: data->blow = inb(io->port); break;
      case 2: data->wlow = inw(io->port); break;
      case 4: data->raw  = inl(io->port); break;
      }
   else
      switch(io->sz)
      {
      case 1: __io_pic_detect(io, data->blow); outb(data->blow, io->port); break;
      case 2: outw(data->wlow, io->port); break;
      case 4: outl(data->raw,  io->port); break;
      }

   return 1;
}

/*
** Faults checks are done in vm_mem access
*/
int dev_io_native(io_insn_t *io, void *device)
{
   emulate_native();
   if(io->s)
      return __io_string_native(io, device);

   return __io_simple_native(io, device);
}

int dev_io_insn(io_insn_t *io, void *device, io_size_t *sz)
{
   sz->done = 0;
   sz->miss = io->sz;

   if(io->s)
      return __io_insn_string(io, device, sz);

   return __io_insn_simple(io, device, sz);
}

static int __dev_io_proxy_filter_in(io_insn_t    *io,
				    io_flt_hdl_t filter,  void      *arg,
				    void         *device, io_size_t *sz)
{
   if(!dev_io_native(io, device))
      return 0;

   debug(DEV_IO, "proxy io in 0x%x = 0x%x\n", io->port, *(uint8_t*)device);

   if(filter && (filter(device, arg) & (VM_FAIL|VM_FAULT)))
      return 0;

   return dev_io_insn(io, device, sz);
}

static int __dev_io_proxy_filter_out(io_insn_t    *io,
				     io_flt_hdl_t filter,  void      *arg,
				     void         *device, io_size_t *sz)
{
   int rc = 1;

   if(!dev_io_insn(io, device, sz))
      return 0;

   if(filter)
   {
      int rc = filter(device, arg);

      if(rc & (VM_FAIL|VM_FAULT))
	 return 0;
   }

   debug(DEV_IO, "proxy io out 0x%x = 0x%x\n", io->port, *(uint8_t*)device);

   /* filter by-pass native io (put here for debug message) */
   if(rc != VM_IGNORE)
      dev_io_native(io, device);

   return 1;
}

int dev_io_proxify_filter(io_insn_t *io, io_flt_hdl_t filter, void *arg)
{
   uint64_t  space;
   loc_t     device;
   io_size_t sz;
   int       rc;

   sz.available = io->sz * io->cnt;

   if(sz.available <= sizeof(uint64_t))
      device.u64 = &space;
   else if(sz.available <= PAGE_SIZE)
   {
      device.linear = pool_pop_page();
      if(!device.linear)
      {
	 debug(DEV_IO, "proxy io memory no more page\n");
	 return 0;
      }
   }
   else
   {
      debug(DEV_IO, "proxy io memory need %D\n", sz.available);
      return 0;
   }

   if(io->in)
      rc = __dev_io_proxy_filter_in(io, filter, arg, device.addr, &sz);
   else
      rc = __dev_io_proxy_filter_out(io, filter, arg, device.addr, &sz);

   if(device.u64 != &space)
      pool_push_page(device.linear);

   return rc;
}

int dev_io_proxify(io_insn_t *io)
{
   return dev_io_proxify_filter(io, NULL, NULL);
}
