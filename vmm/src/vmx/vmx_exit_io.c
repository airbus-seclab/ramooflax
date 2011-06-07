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
#include <vmx_exit_io.h>
#include <vmx_vmcs_acc.h>
#include <info_data.h>
#include <device.h>
#include <debug.h>

extern info_data_t *info;

int vmx_vmexit_resolve_io()
{
   vmcs_exit_info_t *exit  = exit_info(info);
   vmcs_guest_t     *state = guest_state(info);

   if( ! dev_access() )
      return 0;

   vmcs_read( exit->vmexit_insn_len );
   state->rip.low += exit->vmexit_insn_len.raw;
   vmcs_dirty( state->rip );

   return 1;
}

void __vmx_io_init(io_insn_t *io)
{
   vmx_io_t         *vmx;
   vmcs_exit_info_t *exit = exit_info(info);

   vmcs_read( exit->qualification );
   vmx = (vmx_io_t*)&exit->qualification.io_insn;

   io->sz   = vmx->sz + 1;
   io->d    = vmx->d;
   io->s    = vmx->s;
   io->rep  = vmx->rep;
   io->port = vmx->port;
}

int __vmx_io_setup(io_insn_t *io)
{
   return 0;
/*
   vmcs_read( state->ds.selector );
   vmcs_read( state->es.selector );
   vmcs_read( state->cs.base_addr );
   vmcs_read( state->ds.base_addr );
   vmcs_read( state->es.base_addr );
*/

/*
   vmcs_guest_reg_state_t  *reg_st;
   vmcs_vm_exec_ctl_t      *ctls;
   uint32_t                insn_sz;

   reg_st  = &vm->arch.vmcs_region->guest_state.reg_state;
   ctls    = &vm->arch.vmcs_region->controls.exec_ctls;

   io->bck = reg_st->rflags.fields.df;
   io->es  = (uint32_t)reg_st->es.base_addr.low;
   io->ds  = (uint32_t)reg_st->ds.base_addr.low;
   io->pg  = ctls->cr0_read_shadow.fields.pg;
   io->seg = 0;

   io->src.linear = (uint32_t)reg_st->cs.base_addr.low + reg_st->rip.low;
   insn_sz        = vm->arch.vmcs_region->exit_info.vmexit_insn_len;

   if( ! io->pg )
   {
      io->src.linear += (uint32_t)vm->mem.pmem;
      io->dst.linear = io->src.linear + insn_sz;

      if( ! ctls->cr0_read_shadow.fields.pe )
	 io->mask = 0xffff;
      else
	 io->mask = 0xffffffff;
   }
   else
   {
      io->mask = 0xffffffff;
      io->dst.linear = io->src.linear + insn_sz;
      io->src.linear = vm_host_vaddr( vm, io->src.linear );
      io->dst.linear = vm_host_vaddr( vm, io->dst.linear );

      if( (io->dst.linear - io->src.linear) != insn_sz )
      {
	 DEBUG( VMX_DBG, "non contiguous memory !\n" );
	 return 0;
      }
   }

   while( io->src.linear < io->dst.linear )
   {
      if( *io->src.u8 == X86_PREFIX_ADDR )
      {
	 if( ! ctls->cr0_read_shadow.fields.pe )
	    io->mask = 0xffffffff;
	 else
	    io->mask = 0xffff;
      }
      else if( ! io->d && *io->src.u8 == X86_PREFIX_ES )
	 io->seg = 1;

      io->src.u8++;
   }

   return 1;
*/
}
