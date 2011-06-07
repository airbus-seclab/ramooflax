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
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

/* static void __io_insn_string_in(io_insn_t *io, io_size_t *sz) */
/* { */
/*    uint32_t update = info->vm.cpu.gpr->rdi.low & io->mask; */

/*    if(io->bck) */
/*    { */
/*       __io_insn_string_in_bwd(io->dst, io->src, sz, io->sz); */
/*       update -= sz->done & io->mask; */
/*    } */
/*    else */
/*    { */
/*       __io_insn_string_in_fwd(io->dst, io->src, sz, io->sz); */
/*       update += sz->done & io->mask; */
/*    } */

/*    info->vm.cpu.gpr->rdi.low &= ~io->mask; */
/*    info->vm.cpu.gpr->rdi.low |= update; */
/* } */

/* static void __io_insn_string_out(io_insn_t *io, io_size_t *sz) */
/* { */
/*    uint32_t update = info->vm.cpu.gpr->rsi.low & io->mask; */

/*    if(io->bck) */
/*    { */
/*       __io_insn_string_out_bwd(io->dst, io->src, sz, io->sz); */
/*       update -= sz->done & io->mask; */
/*    } */
/*    else */
/*    { */
/*       __io_insn_string_out_fwd(io->dst, io->src, sz, io->sz); */
/*       update += sz->done & io->mask; */
/*    } */

/*    info->vm.cpu.gpr->rsi.low &= ~io->mask; */
/*    info->vm.cpu.gpr->rsi.low |= update; */
/* } */

/* /\* */
/* ** XXX: need security checks */
/* *\/ */
/* static int __io_setup(io_insn_t *io, void *device) */
/* { */
/*    if(!___io_setup(io)) */
/*       return 0; */

/*    if(io->d) */
/*    { */
/*       io->src.addr   = device; */
/*       io->dst.linear = io->es + (info->vm.cpu.gpr->edi.raw & io->mask); */

/*       if(!io->pg) */
/*       	 io->dst.linear += (uint32_t)vm->mem.pmem; */
/*       else */
/* 	 io->dst.linear = vm_host_vaddr(vm, io->dst.linear); */
/*    } */
/*    else */
/*    { */
/*       io->dst.addr   = device; */
/*       io->src.linear = vm->ctx->esi & io->mask; */

/*       if(io->seg) */
/* 	 io->src.linear += io->es; */
/*       else */
/* 	 io->src.linear += io->ds; */

/*       if(!io->pg) */
/* 	 io->src.linear += (uint32_t)vm->mem.pmem; */
/*       else */
/* 	 io->src.linear = vm_host_vaddr(vm, io->src.linear); */
/*    } */

/*    return 1; */
/* } */

/* static int __io_insn_string(io_insn_t *io, void *device, io_size_t *sz) */
/* { */
/*    if(!__io_setup(io, device)) */
/*       return 0; */

/*    if(io->rep) */
/*       sz->miss *= (info->vm.cpu.gpr->rcx.low & io->mask); */

/*    if(io->d) */
/*       __io_insn_string_in(io, sz); */
/*    else */
/*       __io_insn_string_out(io, sz); */

/*    if(io->rep) */
/*       info->vm.cpu.gpr->rcx.low = */
/* 	 (info->vm.cpu.gpr->rcx.low & ~io->mask) | */
/* 	 ((info->vm.cpu.gpr->rcx.low & io->mask) - (((sz->done)/io->sz) & io->mask)); */

/*    if(sz->miss) */
/*    { */
/*       debug(DEV_IO, "missing %d bytes, available %d, done %d !\n", */
/* 	    sz->miss, sz->available, sz->done); */
/*       return 0; */
/*    } */

/*    return 1; */
/* } */

static int __io_insn_simple(io_insn_t *io, void *device, io_size_t *sz)
{
   loc_t  dst, src;

   if(io->d)
   {
      dst.addr = (void*)&info->vm.cpu.gpr->rax.low;
      src.addr = device;
   }
   else
   {
      dst.addr = device;
      src.addr = (void*)&info->vm.cpu.gpr->rax.low;
   }

   switch(io->sz)
   {
   case 1:
      __io_insn_byte(dst, src);
      break;
   case 2:
      __io_insn_word(dst, src);
      break;
   case 4:
      __io_insn_long(dst, src);
      break;
   }

   sz->done      += io->sz;
   sz->available -= io->sz;
   sz->miss      -= io->sz;

   return 1;
}

int dev_io_insn(io_insn_t *io, void *device, io_size_t *sz)
{
   if(io->sz > sz->available)
   {
      debug(DEV_IO, "not enough space for i/o op size\n");
      return 0;
   }

   sz->done = 0;
   sz->miss = io->sz;

   if(io->s)
      panic("io string emulation not implemented");
      /* return __io_insn_string(io, device, sz); */

   return __io_insn_simple(io, device, sz);
}
