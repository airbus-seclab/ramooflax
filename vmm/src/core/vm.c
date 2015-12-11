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
** VM internal memory access operators
*/
static int __vm_access_local_operator(vm_access_t *access)
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
   access->done += access->len;

#ifdef CONFIG_VM_ACCESS_DBG
   if(!__rmode())
   {
      debug(VM_ACCESS,
	    "vm_access_local %s dst 0x%X src 0x%X ln %D [0x%x:0x%x]\n"
	    ,access->wr ? "write":"read"
	    ,dst.linear, src.linear, access->len
	    ,dst.u8[0], dst.u8[access->len -1]);
      size_t i=0;
      while(i<access->len)
	 printf("%c", dst.u8[i++]);
      printf("\n");
   }
#endif

   return VM_DONE;
}

static int __vm_access_remote_operator(vm_access_t *access)
{
   loc_t  loc;
   size_t done, len;

   loc.linear = access->addr;

   if(access->wr)
   {
      ctrl_io_write(loc.u8, access->len);
      access->done += access->len;
      return VM_DONE;
   }

   len = access->len;
   while(len)
   {
      done = ctrl_io_read(loc.u8, len);

      loc.linear   += done;
      access->done += done;
      len          -= done;
   }

   return VM_DONE;
}

static int __vm_access_smem_identity(vm_access_t *access)
{
   /*
   ** identity mapped guest paddr to system addr
   ** so no walk through npg tables to be faster
   **
   ** we must check for scarry access
   */
   if(vmm_area_range(access->addr, access->len))
      return VM_FAIL;

   return access->operator(access);
}

#define __vm_access_smem(_a_) __vm_access_smem_identity(_a_)

static int __vm_check_pmem(vm_access_t __unused__ *access)
{
   /* XXX
   ** - segment limit
   ** - segment valid
   ** - align check
   */
   return VM_DONE;
}

static int __vm_access_pmem(vm_access_t *access)
{
   if(!access->len)
      return VM_DONE;

   if(__vm_check_pmem(access) != VM_DONE)
      return VM_FAIL;

   return __vm_access_smem(access);
}

static int __vm_access_vmem(vm_access_t *access)
{
   pg_wlk_t wlk;
   offset_t vaddr, nxt;
   size_t   len;
   int      rc;

   if(!__paging())
      return __vm_access_pmem(access);

   vaddr = access->addr;
   len = access->len;

   while(len)
   {
      rc = __pg_walk(access->cr3, vaddr, &wlk);
      if(rc != VM_DONE)
      {
	 debug(VM, "#PF on vm access 0x%X sz 0x%X\n", vaddr, len);
	 if(rc == VM_FAULT)
	 {
	    pf_err_t pf = {.raw = 0};

	    if(access->rq == VM_ACC_REQ_VMM)
	       return VM_PARTIAL;

	    pf.wr = access->wr;
	    pf.us = (__cpl == 3);
	    __inject_exception(PF_EXCP, pf.raw, vaddr);
	 }

	 return rc;
      }

      nxt = __align_next(vaddr, wlk.size);
      access->addr = wlk.addr;
      access->len  = min(len, (nxt - vaddr));

      rc = __vm_access_pmem(access);
      if(rc != VM_DONE)
	 return rc;

      len  -= access->len;
      vaddr = nxt;
   }

   return VM_DONE;
}

static void __vm_access_setup(vm_access_t *access,
			      cr3_reg_t *cr3, offset_t addr,
			      uint8_t *data, size_t len,
			      uint8_t wr, uint8_t rq, uint8_t rm)
{
   access->addr = addr;
   access->data = (void*)data;
   access->len  = len;
   access->done = 0;
   access->wr   = wr;
   access->rq   = rq;

   if(cr3)
      access->cr3  = cr3;

   if(rm)
      access->operator = __vm_access_remote_operator;
   else
      access->operator = __vm_access_local_operator;
}

/*
** VM physical memory access operators for VMM operations only
*/
int __vm_recv_pmem(offset_t paddr, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, NULL, paddr, NULL, len, 0, VM_ACC_REQ_VMM, 1);
   return __vm_access_pmem(&access);
}

int __vm_send_pmem(offset_t paddr, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, NULL, paddr, NULL, len, 1, VM_ACC_REQ_VMM, 1);
   return __vm_access_pmem(&access);
}

int __vm_read_pmem(offset_t paddr, uint8_t *data, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, NULL, paddr, data, len, 0, VM_ACC_REQ_VMM, 0);
   return __vm_access_pmem(&access);
}

int __vm_write_pmem(offset_t paddr, uint8_t *data, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, NULL, paddr, data, len, 1, VM_ACC_REQ_VMM, 0);
   return __vm_access_pmem(&access);
}

/*
** VM virtual memory access operators for VMM operations only
*/
int __vm_recv_vmem(cr3_reg_t *cr3, offset_t vaddr, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, cr3, vaddr, NULL, len, 0, VM_ACC_REQ_VMM, 1);
   return __vm_access_vmem(&access);
}

int __vm_send_vmem(cr3_reg_t *cr3, offset_t vaddr, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, cr3, vaddr, NULL, len, 1, VM_ACC_REQ_VMM, 1);
   return __vm_access_vmem(&access);
}

int __vm_read_vmem(cr3_reg_t *cr3, offset_t vaddr, uint8_t *data, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, cr3, vaddr, data, len, 0, VM_ACC_REQ_VMM, 0);
   return __vm_access_vmem(&access);
}

int __vm_write_vmem(cr3_reg_t *cr3, offset_t vaddr, uint8_t *data, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, cr3, vaddr, data, len, 1, VM_ACC_REQ_VMM, 0);
   return __vm_access_vmem(&access);
}

/*
** VM virtual memory access operators upon VM request
*/
int vm_read_mem_sz(offset_t vaddr, uint8_t *data, size_t len, size_t *done)
{
   int         rc;
   vm_access_t access;

   __vm_access_setup(&access, &__cr3, vaddr, data, len, 0, VM_ACC_REQ_VM, 0);
   rc = __vm_access_vmem(&access);
   *done = access.done;
   return rc;
}

int vm_write_mem_sz(offset_t vaddr, uint8_t *data, size_t len, size_t *done)
{
   int         rc;
   vm_access_t access;

   __vm_access_setup(&access, &__cr3, vaddr, data, len, 1, VM_ACC_REQ_VM, 0);
   rc = __vm_access_vmem(&access);
   *done = access.done;
   return rc;
}

int vm_read_mem(offset_t vaddr, uint8_t *data, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, &__cr3, vaddr, data, len, 0, VM_ACC_REQ_VM, 0);
   return __vm_access_vmem(&access);
}

int vm_write_mem(offset_t vaddr, uint8_t *data, size_t len)
{
   vm_access_t access;
   __vm_access_setup(&access, &__cr3, vaddr, data, len, 1, VM_ACC_REQ_VM, 0);
   return __vm_access_vmem(&access);
}

/*
** Resolve guest virtual into guest physical
*/
int vm_pg_walk(offset_t vaddr, pg_wlk_t *wlk)
{
   if(!__paging())
   {
      debug(VM, "vm page walk while paging disabled !\n");
      return VM_FAIL;
   }

   return pg_walk(vaddr, wlk);
}

/*
** Resolve guest virtual into system physical
**/
int vm_full_walk(offset_t vaddr, pg_wlk_t *wlk)
{
   int rc = vm_pg_walk(vaddr, wlk);

   if(rc == VM_DONE)
      return npg_walk(wlk->addr, wlk);

   return rc;
}

/*
** Change active Nested Paging
*/
void vm_setup_npg(int who)
{
   npg_set_active_paging(who);
   npg_set_active_paging_cpu();
}

/*
** VM cpu mode transitions
*/
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
