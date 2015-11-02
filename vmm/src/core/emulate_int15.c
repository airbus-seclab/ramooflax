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
#include <emulate.h>
#include <intr.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** A20 gate disable: AX = 2400h
** CF clear if successful
** AH = 00h
** CF set on error
** AH = status
** 01h keyboard controller is in secure mode
** 86h function not supported
*/
static int emulate_int15_a20_disable()
{
   info->vm.cpu.gpr->rax.bhigh = 0;

   __rflags.cf = 0;
   __post_access(__rflags);

   dev_a20_set(0);

   return VM_DONE;
}

/*
** A20 gate enable: AX = 2401h
** CF clear if successful
** AH = 00h
** CF set on error
** AH = status
** 01h  keyboard controller is in secure mode
** 86h function not supported
*/
static int emulate_int15_a20_enable()
{
   info->vm.cpu.gpr->rax.bhigh = 0;

   __rflags.cf = 0;
   __post_access(__rflags);

    dev_a20_set(1);

   return VM_DONE;
}

/*
** A20 gate status: AX = 2402h
** CF clear if successful
** AH = 00h
** AL = current state (00h disabled, 01h enabled)
** CX = ??? (set to 0000h-000Bh or FFFFh by AMI BIOS v1.00.03.AV0M)
**      FFFFh if keyboard controller does not become ready
**      within C000h read attempts
** CF set on error
** AH = status 01h
** keyboard controller is in secure mode
** 86h function not supported
*/
static int emulate_int15_a20_status()
{
   info->vm.cpu.gpr->rax.blow  = info->vm.dev.mem.a20;
   info->vm.cpu.gpr->rax.bhigh = 0;
   info->vm.cpu.gpr->rcx.wlow  = 0;

   __rflags.cf = 0;
   __post_access(__rflags);

   return VM_DONE;
}

/*
** A20 gate support: AX = 2403h
** CF clear if successful
** AH = 00h
** BX = status of A20 gate support
**     0 supported on keyboard controller
**     1 supported with bit 1 of I/O port 92h
**  14-2 reserved
**    15 additional data is available (location not yet defined)
** CF set on error
** AH = status 01h keyboard controller is in secure mode
** 86h function not supported
*/
static int emulate_int15_a20_support()
{
   info->vm.cpu.gpr->rax.bhigh = 0;
   info->vm.cpu.gpr->rbx.wlow  = 2;

   __rflags.cf = 0;
   __post_access(__rflags);

   return VM_DONE;
}

/*
** Report memory above 64MB
**
** AX = extended memory between 1MB and 16MB, in K (max 3C00h = 15MB)
** BX = extended memory above 16MB, in 64KB
** CX = configured memory 1MB to 16MB, in KB
** DX = configured memory above 16MB, in 64KB
** CF clear if successful
** CF set on error
*/
static int emulate_int15_get_ext_mem()
{
   smap_t   *smap = &info->vm.dev.mem.smap;
   smap_e_t *sme  = smap->entries;
   size_t   n;
   raw64_t  x;

   for(n=0 ; n<smap->nr ; n++, sme++)
      if(sme->type == SMAP_TYPE_AVL && sme->base == 0x100000ULL)
      {
	 x.raw = (sme->base + sme->len) - (16<<20);
	 x.raw >>= 16;
	 break;
      }

   if(x.raw == 0)
   {
      debug(EMU_RMODE_INT15, "no extended memory found\n");
      return VM_FAIL;
   }

   info->vm.cpu.gpr->rax.wlow = (15<<10);
   info->vm.cpu.gpr->rbx.wlow = x.wlow;

   info->vm.cpu.gpr->rcx.wlow = info->vm.cpu.gpr->rax.wlow;
   info->vm.cpu.gpr->rdx.wlow = info->vm.cpu.gpr->rbx.wlow;

   __rflags.cf = 0;
   __post_access(__rflags);

   return VM_DONE;
}

/*
** Report memory between 1MB - 16MB
**
** AX = extended memory between 1MB and 16MB, in KB (max 3C00h = 15MB)
** CF clear if successful
** CF set on error
*/
static int emulate_int15_old_get_ext_mem()
{
   info->vm.cpu.gpr->rax.wlow = (15<<10);

   __rflags.cf = 0;
   __post_access(__rflags);

   return VM_DONE;
}

/*
** Emulate SMAP bios query
**
** XXX: complete access control needed !
** - check expand down/up for limit meaning
** - check if >= (seg_limit+1)*unit
** - check segment wrapping
** - check a20 line and real-mode addr
** - check vmm area collision
*/
static int emulate_int15_smap()
{
   loc_t       src, dst;
   offset_t    top_offset, crt_offset, remaining;
   smap_t      *smap;
   gpr64_ctx_t *gpr;

   gpr  = info->vm.cpu.gpr;
   smap = &info->vm.dev.mem.smap;

   if(gpr->rdx.low != BIOS_SMAP_ID)
      return VM_FAULT;

   if(gpr->rcx.low < sizeof(smap_e_t))
      return VM_FAULT;

   if(gpr->rcx.low > sizeof(smap_e_t))
      gpr->rcx.low = sizeof(smap_e_t);

   crt_offset = (offset_t)gpr->rbx.low;
   top_offset = (offset_t)(smap->nr * sizeof(smap_e_t));

   if(crt_offset >= top_offset)
   {
      gpr->rbx.low = 0;
      gpr->rcx.low = 0;
      return VM_DONE;
   }

   src.linear = (offset_t)smap->raw + crt_offset;
   remaining = top_offset - crt_offset;

   if(remaining <= sizeof(smap_e_t))
   {
      gpr->rbx.low = 0;
      gpr->rcx.low = (uint32_t)remaining;
   }
   else
      gpr->rbx.low += gpr->rcx.low;

   __pre_access(__es.base);
   dst.linear = __es.base.low + gpr->rdi.wlow;

   if(vmm_area_range(dst.linear, gpr->rcx.low))
   {
      debug(SMAP, "vm stores smap entry into vmm area\n");
      return VM_FAIL;
   }

   memcpy(dst.addr, src.addr, gpr->rcx.low);
   return VM_DONE;
}

int emulate_int15()
{
   raw64_t *rax = &info->vm.cpu.gpr->rax;

   switch(rax->wlow)
   {
   case BIOS_GET_SMAP:
      debug(SMAP, "get smap\n");
      switch(emulate_int15_smap())
      {
      case VM_DONE:
	 __rflags.cf = 0;
	 rax->low = BIOS_SMAP_ID;
	 break;
      case VM_FAULT:
	 __rflags.cf = 1;
	 rax->bhigh = BIOS_SMAP_ERROR;
	 break;
      default:
	 return VM_FAIL;
      }
      __post_access(__rflags);
      return VM_DONE;

   case BIOS_GET_EXT_MEM_32:
   case BIOS_GET_EXT_MEM:
      debug(EMU_RMODE_INT15, "get ext mem\n");
      return emulate_int15_get_ext_mem();
   case BIOS_DISABLE_A20:
      debug(EMU_RMODE_INT15, "disable a20\n");
      return emulate_int15_a20_disable();
   case BIOS_ENABLE_A20:
      debug(EMU_RMODE_INT15, "enable a20\n");
      return emulate_int15_a20_enable();
   case BIOS_STATUS_A20:
      debug(EMU_RMODE_INT15, "status a20\n");
      return emulate_int15_a20_status();
   case BIOS_SUPPORT_A20:
      debug(EMU_RMODE_INT15, "support a20\n");
      return emulate_int15_a20_support();
   }

   switch(rax->bhigh)
   {
   case BIOS_OLD_GET_EXT_MEM:
      debug(EMU_RMODE_INT15, "get old ext mem\n");
      return emulate_int15_old_get_ext_mem();
   case BIOS_GET_BIG_MEM:
      debug(EMU_RMODE_INT15, "get big mem !\n");
      return VM_FAIL;
   }

   return VM_DONE_LET_RIP;
}
