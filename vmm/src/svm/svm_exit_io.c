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
#include <svm_exit_io.h>
#include <info_data.h>
#include <dev.h>
#include <debug.h>

extern info_data_t *info;

int svm_vmexit_resolve_io()
{
   vmcb_ctrls_area_t *ctrls = &info->vm.cpu.vmc->vm_vmcb.ctrls_area;
   vmcb_state_area_t *state = &info->vm.cpu.vmc->vm_vmcb.state_area;

   if(!dev_access())
      return 0;

   state->rip.raw = ctrls->exit_info_2.raw;
   return 1;
}

void __svm_io_init(io_insn_t *io)
{
   svm_io_t *svm = (svm_io_t*)&info->vm.cpu.vmc->vm_vmcb.ctrls_area.exit_info_1;

   io->sz   = (svm->low>>4) & 7;
   io->mask = (svm->low>>7) & 7;

   io->d    = svm->io.d;
   io->s    = svm->io.s;
   io->rep  = svm->io.rep;
   io->port = svm->io.port;
}

/* int __svm_io_setup(io_insn_t *io) */
/* { */
/*    vmcb_state_area_t *state; */
/*    vmcb_ctrls_area_t *ctrls; */

/*    state     = &vm->arch.vmcb->state_area; */
/*    ctrls     = &vm->arch.vmcb->ctrls_area; */

/*    io->bck   = state->rflags.fields.df; */
/*    io->es    = state->es.base_addr.low; */
/*    io->ds    = state->ds.base_addr.low; */
/*    io->pg    = svm_paging(vm); */
/*    io->seg   = 0; */

/*    if( io->mask == 1 ) */
/*       io->mask = 0xffff; */
/*    else if( io->mask == 2 ) */
/*       io->mask = 0xffffffff; */
/*    else */
/*    { */
/*       debug( SVM, "64 bits addr unsupported\n" ); */
/*       return 0; */
/*    } */

/*    if( ! io->d ) */
/*    { */
/*       uint32_t insn_sz = ctrls->exit_info_2.low - state->rip.low; */

/*       io->src.linear = (uint32_t)state->cs.base_addr.low + state->rip.low; */

/*       if( ! io->pg ) */
/*       { */
/* 	 io->src.linear += (uint32_t)vm->mem.pmem; */
/* 	 io->dst.linear = io->src.linear + insn_sz; */
/*       } */
/*       else */
/*       { */
/* 	 io->dst.linear = io->src.linear + insn_sz; */
/* 	 io->dst.linear = vm_host_vaddr( vm, io->dst.linear ); */
/* 	 io->src.linear = vm_host_vaddr( vm, io->src.linear ); */

/* 	 if( (io->dst.linear - io->src.linear) != insn_sz ) */
/* 	 { */
/* 	    debug( SVM, "non contiguous memory !\n" ); */
/* 	    return 0; */
/* 	 } */
/*       } */

/*       while( io->src.linear < io->dst.linear ) */
/*       { */
/* 	 if( *io->src.u8 == X86_PREFIX_ES ) */
/* 	 { */
/* 	    io->seg = 1; */
/* 	    break; */
/* 	 } */
/* 	 io->src.u8++; */
/*       } */
/*    } */
/*    return 1; */
/* } */
