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
#include <vm.h>
#include <paging.h>
#include <intr.h>
#include <debug.h>
#include <ctrl_io.h>
#include <dev_io_ports.h>
#include <info_data.h>

extern info_data_t *info;

/*
** Get default addr size
** take care of long/compat/legacy mode
**
** notice that instruction addr prefix (\x67)
** can change default size
*/
static void __vm_resolve_seg_offset(offset_t *vaddr, offset_t base, offset_t offset,
				    offset_t addend, int *mode)
{
   *mode = cpu_addr_sz();

   if(*mode == 64)
      *vaddr = offset + addend;
   else
   {
      *vaddr = (base & 0xffffffff);

      if(*mode == 32)
	 *vaddr += (offset & 0xffffffff) + (addend & 0xffffffff);
      else
	 *vaddr += (offset & 0xffff) + (addend & 0xffff);
   }
}

void vm_get_code_addr(offset_t *vaddr, offset_t addend, int *mode)
{
   __vm_resolve_seg_offset(vaddr, __cs.base.raw, __rip.raw, addend, mode);
}

void vm_get_stack_addr(offset_t *vaddr, offset_t addend, int *mode)
{
   __vm_resolve_seg_offset(vaddr, __ss.base.raw, info->vm.cpu.gpr->rsp.raw,
			   addend, mode);
}

void vm_update_rip(offset_t offset)
{
   int mode = cpu_addr_sz();
   if(mode == 64)
      __rip.raw += (uint64_t)offset;
   else if(mode == 32)
      __rip.low += (uint32_t)offset;
   else
      __rip.wlow += (uint16_t)offset;

   __post_access(__rip);
}

void vm_rewind_rip(offset_t offset)
{
   int mode = cpu_addr_sz();
   if(mode == 64)
      __rip.raw -= (uint64_t)offset;
   else if(mode == 32)
      __rip.low -= (uint32_t)offset;
   else
      __rip.wlow -= (uint16_t)offset;

   __post_access(__rip);
}

/*
** VM memory access operators
*/
static int __vm_local_access_pmem(vm_access_t *access)
{
   loc_t src, dst;

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
   return VM_DONE;
}

int __vm_remote_access_pmem(vm_access_t *access)
{
   loc_t  loc;
   size_t done, len;

   loc.linear = access->addr;

   if(access->wr)
   {
      ctrl_io_write(loc.u8, access->len);
      return VM_DONE;
   }

   len = access->len;
   while(len)
   {
      done = ctrl_io_read(loc.u8, len);
      loc.linear += done;
      len -= done;
   }

   return VM_DONE;
}

static int __vm_access_smem(vm_access_t *access)
{
   npg_wlk_t   wlk;
   npg_pte64_t *pte;

   if(npg_walk(access->addr, &wlk) != VM_DONE)
      return VM_FAIL;

   /* pdpe, pde, pte have same layout for pvl checking */
   pte = (npg_pte64_t*)wlk.entry;

   /* XXX
   ** check access on several nested entries if granularity
   ** is finest than guest virtual mapping (__vm_access_vmem)
   **
   ** ie. guest  has 2MB entry
   **     nested has several 4KB entries some of them not mapped
   */
   if(access->len > wlk.size)
   {
      debug(VM_ACCESS, "vm_access: need to check several npg entries\n");
      return VM_FAIL;
   }

   if(access->wr)
   {
      if(!npg_writable(pte))
#ifdef CONFIG_SNAPSHOT
	 if(__ctrl_snapshot_npg_cow(&wlk, access->addr) != VM_DONE)
#endif
	    return VM_FAIL;
   }
   else if(!npg_readable(pte))
      return VM_FAIL;

   access->addr = wlk.addr;
   return VM_DONE;
}

static int __vm_check_pmem(vm_access_t __unused__ *access)
{
   /*
   ** XXX
   ** - segment limit
   ** - segment valid
   ** - align check
   */
   return VM_DONE;
}

static int __vm_access_pmem(vm_access_t *access)
{
   if(vmm_area_range(access->addr, access->len))
      return VM_FAIL;

   if(__vm_check_pmem(access) != VM_DONE)
      return VM_FAIL;

   if(__vm_access_smem(access) != VM_DONE)
      return VM_FAIL;

   return access->operator(access);
}

static int __vm_access_vmem(vm_access_t *access)
{
   offset_t vaddr, nxt;
   size_t   psz, len, olen;

   if(!access->len)
      return VM_DONE;

   if(!__paging())
      return access->operator(access);

   vaddr = access->addr;
   olen = len = access->len;

   while(len)
   {
      if(!__pg_walk(access->cr3, vaddr, &access->addr, &psz, 1))
      {
	 debug(VM_ACCESS, "#PF on vm access 0x%X sz 0x%X\n", vaddr, len);
	 goto __vm_access_error;
      }

      nxt = __align_next(vaddr, psz);
      access->len = min(len, (nxt - vaddr));

      if(__vm_access_pmem(access) != VM_DONE)
	 goto __vm_access_error;

      len  -= access->len;
      vaddr = nxt;
   }

   return VM_DONE;

__vm_access_error:
   if(len != olen)
      return VM_PARTIAL;

   return VM_FAIL;
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

   return __vm_access_vmem(&access);
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

   return __vm_access_vmem(&access);
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

   return __vm_access_vmem(&access);
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

   return __vm_access_vmem(&access);
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

   return __vm_access_vmem(&access);
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

   return __vm_access_vmem(&access);
}

int vm_enter_pmode()
{
   info->vm.cpu.dflt_excp = VM_PMODE_EXCP_BITMAP;
   __update_exception_mask();

   __allow_soft_int();
   __allow_io_range(KBD_START_PORT, KBD_END_PORT);

   return VM_DONE;
}

int vm_enter_rmode()
{
   info->vm.cpu.dflt_excp = VM_RMODE_EXCP_BITMAP;
   __update_exception_mask();

   __deny_soft_int();
   __deny_io_range(KBD_START_PORT, KBD_END_PORT);

   return VM_DONE;
}

void vm_setup_npg(int who)
{
   npg_set_active_paging(who);
   npg_set_active_paging_cpu();
}

/*
** Resolve guest virtual into guest physical
*/
int vm_pg_walk(offset_t vaddr, offset_t *paddr, size_t *psz)
{
   if(!__paging())
   {
      debug(VM, "walk while paging disabled !\n");
      return VM_FAIL;
   }

   return (__pg_walk(&__cr3, vaddr, paddr, psz, 1) ? VM_DONE : VM_FAIL);
}

/*
** Resolve guest virtual into system physical
**/
int vm_full_walk(offset_t vaddr, npg_wlk_t *wlk)
{
   size_t   sz;
   offset_t gp;

   if(vm_pg_walk(vaddr, &gp, &sz) == VM_DONE)
      return npg_walk(gp, wlk);

   return VM_FAIL;
}
