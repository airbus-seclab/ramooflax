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
#include <vm.h>
#include <paging.h>
#include <dev_io_ports.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** Take care of lmode/compatmode/legacymode differences
** cf. table-2.6 (page 39) of amd manual
** cf. table 14-4 (page 358) of amd manual
**
** notice that instruction prefix can change these defaults
** addr/operand sizes
*/
static void __vm_resolve_seg_offset(offset_t *vaddr, offset_t base, offset_t offset,
				    offset_t addend, int *mode)
{
   if(__lmode64())
   {
      *mode = 64;
      *vaddr = offset + addend;
   }
   else
   {
      *vaddr = (base & 0xffffffff);

      if(__pmode32())
      {
	 *mode = 32;
	 *vaddr += (offset & 0xffffffff) + (addend & 0xffffffff);
      }
      else
      {
	 *mode = 16;
	 *vaddr += (offset & 0xffff) + (addend & 0xffff);
      }
   }
}

void vm_get_code_addr(offset_t *vaddr, offset_t addend, int *mode)
{
   __vm_resolve_seg_offset(vaddr, __cs.base_addr.raw, __rip.raw, addend, mode);
}

void vm_get_stack_addr(offset_t *vaddr, offset_t addend, int *mode)
{
   __vm_resolve_seg_offset(vaddr, __ss.base_addr.raw, info->vm.cpu.gpr->rsp.raw,
			   addend, mode);
}

void vm_update_rip(offset_t offset)
{
   if(__lmode64())
      __rip.raw += (uint64_t)offset;
   else if(__pmode32())
      __rip.low += (uint32_t)offset;
   else
      __rip.wlow += (uint16_t)offset;

   __post_access(__rip);
}

void vm_rewind_rip(offset_t offset)
{
   if(__lmode64())
      __rip.raw -= (uint64_t)offset;
   else if(__pmode32())
      __rip.low -= (uint32_t)offset;
   else
      __rip.wlow -= (uint16_t)offset;

   __post_access(__rip);
}

int __vm_local_access_pmem(vm_access_t *access)
{
   loc_t src, dst;

   if(vmm_area_range(access->addr, access->len))
      return 0;

   if(access->wr)
   {
      dst.linear = access->addr;
      src.addr = access->data;
   }
   else
   {
      dst.addr = access->data;
      src.linear = access->addr;
   }

   memcpy(dst.addr, src.addr, access->len);
   return 1;
}

int __vm_remote_access_pmem(vm_access_t *access)
{
   loc_t  loc;
   size_t done, len;
   
   if(vmm_area_range(access->addr, access->len))
      return 0;

   loc.linear = access->addr;

   if(access->wr)
   {
      ctrl_write(loc.u8, access->len);
      return 1;
   }

   len = access->len;
   while(len)
   {
      done = ctrl_read(loc.u8, len);
      loc.linear += done;
      len -= done;
   }

   return 1;
}

int __vm_access_mem(vm_access_t *access)
{
   offset_t vaddr, nxt;
   size_t   psz, len;

   if(!access->len)
      return 1;

   if(!__paging())
      return access->operator(access);

   vaddr = access->addr;
   len   = access->len;

   while(len)
   {
      if(!__pg_walk(access->cr3, vaddr, &access->addr, &psz))
      {
	 debug(VM_ACCESS, "#PF on vm access 0x%X sz 0x%X\n", vaddr, len);
	 return 0;
      }

      nxt = __align_next(vaddr, psz);
      access->len = min(len, (nxt - vaddr));

      if(!access->operator(access))
	 return 0;

      len  -= access->len;
      vaddr = nxt;
   }

   return 1;
}

int __vm_recv_mem(cr3_reg_t *cr3, offset_t vaddr, size_t len)
{
   vm_access_t access;

   access.cr3  = cr3;
   access.addr = vaddr;
   access.data = (void*)0;
   access.len  = len;
   access.wr   = 0;
   access.operator = __vm_remote_access_pmem;

   return __vm_access_mem(&access);
}

int __vm_send_mem(cr3_reg_t *cr3, offset_t vaddr, size_t len)
{
   vm_access_t access;

   access.cr3  = cr3;
   access.addr = vaddr;
   access.data = (void*)0;
   access.len  = len;
   access.wr   = 1;
   access.operator = __vm_remote_access_pmem;

   return __vm_access_mem(&access);
}

int __vm_read_mem(cr3_reg_t *cr3, offset_t vaddr, uint8_t *data, size_t len)
{
   vm_access_t access;

   access.cr3  = cr3;
   access.addr = vaddr;
   access.data = (void*)data;
   access.len  = len;
   access.wr   = 0;
   access.operator = __vm_local_access_pmem;

   return __vm_access_mem(&access);
}

int __vm_write_mem(cr3_reg_t *cr3, offset_t vaddr, uint8_t *data, size_t len)
{
   vm_access_t access;

   access.cr3  = cr3;
   access.addr = vaddr;
   access.data = (void*)data;
   access.len  = len;
   access.wr   = 1;
   access.operator = __vm_local_access_pmem;

   return __vm_access_mem(&access);
}

int vm_read_mem(offset_t vaddr, uint8_t *data, size_t len)
{
   vm_access_t access;

   access.cr3  = &__cr3;
   access.addr = vaddr;
   access.data = (void*)data;
   access.len  = len;
   access.wr   = 0;
   access.operator = __vm_local_access_pmem;

   return __vm_access_mem(&access);
}

int vm_write_mem(offset_t vaddr, uint8_t *data, size_t len)
{
   vm_access_t access;

   access.cr3  = &__cr3;
   access.addr = vaddr;
   access.data = (void*)data;
   access.len  = len;
   access.wr   = 1;
   access.operator = __vm_local_access_pmem;

   return __vm_access_mem(&access);
}

int vm_enter_pmode()
{
   pdpe_t *pdp = info->vm.cpu.pg.pm.pdp[0];

   info->vm.cpu.dflt_excp = VM_PMODE_EXCP_BITMAP;

   __pg_set_entry(&info->vm.cpu.pg.pm.pml4[0], PG_USR|PG_RW, page_nr(pdp));
   __allow_soft_int();
   __allow_io_range(KBD_START_PORT, KBD_END_PORT);

   return 1;
}

int vm_enter_rmode()
{
   pdpe_t *pdp = info->vm.cpu.pg.rm->pdp;

   info->vm.cpu.dflt_excp = VM_RMODE_EXCP_BITMAP;

   __pg_set_entry(&info->vm.cpu.pg.rm->pml4[0], PG_USR|PG_RW, page_nr(pdp));
   __deny_soft_int();
   __deny_io_range(KBD_START_PORT, KBD_END_PORT);

   return 1;
}
