/*
** Copyright (C) 2016 Airbus Group, stephane duverger <stephane.duverger@airbus.com>
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
#include <vmx_ept.h>
#include <npg.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

#ifndef __INIT__

static int vmx_ept_pdpe_to_pae_pdpe(pdpe_t *pdpe, pdpe_t *pae_pdpe)
{
   pae_pdpe->raw = pdpe->raw;
   return valid_pae_pdpe(pae_pdpe);
}

int vmx_ept_update_pdpe()
{
   pdpe_t *pdp = (pdpe_t*)pg_32B_addr((offset_t)__cr3.pae.addr);

   if(  !vmm_area(pdp)
      && vmx_ept_pdpe_to_pae_pdpe(&pdp[0], &vm_state.pdpe_0)
      && vmx_ept_pdpe_to_pae_pdpe(&pdp[1], &vm_state.pdpe_1)
      && vmx_ept_pdpe_to_pae_pdpe(&pdp[2], &vm_state.pdpe_2)
      && vmx_ept_pdpe_to_pae_pdpe(&pdp[3], &vm_state.pdpe_3))
   {
      vmcs_dirty(vm_state.pdpe_0);
      vmcs_dirty(vm_state.pdpe_1);
      vmcs_dirty(vm_state.pdpe_2);
      vmcs_dirty(vm_state.pdpe_3);
      return 1;
   }

   return 0;
}
#endif

uint64_t __ept_mtrr_resolve(uint64_t old, uint64_t new)
{
   uint64_t attr, n_type, o_type, c_type;

   o_type = (old>>3)&7;
   n_type = (new>>3)&7;
   c_type =  o_type & n_type;

   if(o_type == n_type)
      return new;

   if(c_type != VMX_EPT_MEM_TYPE_UC && c_type != VMX_EPT_MEM_TYPE_WT)
      panic("EPT MTRR overlap not managed: 0x%X 0x%X", old, new);

   attr = (new & ~(7ULL<<3)) | (c_type<<3);
   return attr;
}

static void vmx_ept_map_mtrr(offset_t addr, size_t len, uint64_t attr)
{
   offset_t end = addr + len;

   npg_unmap(addr, end);
   npg_map(addr, end, attr);
}

static offset_t vmx_ept_map_mtrr_fixed_unit(offset_t base, size_t unit, msr_t msr)
{
   size_t i;

   for(i=0 ; i<8 ; i++)
   {
      uint8_t  type = (msr.raw>>(i*8)) & 0xff;
      uint64_t attr = ept_has_mtrr|(type<<3)|ept_dft_pvl;

      vmx_ept_map_mtrr(base, unit, attr);
      base += unit;
   }

   return base;
}

static void vmx_ept_map_mtrr_fixed()
{
   msr_t    msr;
   offset_t base;
   size_t   n;

   rd_msr64(IA32_MTRR_FIX64K_00000, msr.edx, msr.eax);
   debug(VMX_EPT, "mtrr fixed 64K [0x%x] = 0x%X\n", IA32_MTRR_FIX64K_00000,msr.raw);
   base = vmx_ept_map_mtrr_fixed_unit(0, 64*1024, msr);

   rd_msr64(IA32_MTRR_FIX16K_80000, msr.edx, msr.eax);
   debug(VMX_EPT, "mtrr fixed 16K [0x%x] = 0x%X\n", IA32_MTRR_FIX16K_80000,msr.raw);
   base = vmx_ept_map_mtrr_fixed_unit(base, 16*1024, msr);

   rd_msr64(IA32_MTRR_FIX16K_a0000, msr.edx, msr.eax);
   debug(VMX_EPT, "mtrr fixed 16K [0x%x] = 0x%X\n", IA32_MTRR_FIX16K_a0000,msr.raw);
   base = vmx_ept_map_mtrr_fixed_unit(base, 16*1024, msr);

   for(n=IA32_MTRR_FIX4K_c0000 ; n<=IA32_MTRR_FIX4K_f8000 ; n++)
   {
      rd_msr64(n, msr.edx, msr.eax);
      debug(VMX_EPT, "mtrr fixed  4K [0x%x] = 0x%X\n", n,msr.raw);
      base = vmx_ept_map_mtrr_fixed_unit(base, 4*1024, msr);
   }
}

static void vmx_ept_map_mtrr_variable()
{
   size_t i;

   for(i=0 ; i<info->vm.mtrr_cap.vcnt ; i++)
   {
      ia32_mtrr_physbase_t m_base;
      ia32_mtrr_physmask_t m_mask;

      rd_msr_ia32_mtrr_physbase(m_base, i);
      rd_msr_ia32_mtrr_physmask(m_mask, i);

      if(m_mask.v)
      {
         offset_t base = m_base.base<<12;
         offset_t mask = m_mask.mask<<12;
         size_t   len  = info->vm.cpu.max_paddr - mask + 1;
         uint64_t attr = ept_has_mtrr|(m_base.type<<3)|ept_dft_pvl;

         debug(VMX_EPT
               ,"mtrr #%d base 0x%X mask 0x%X type %d [0x%X - 0x%X] (len 0x%X)\n"
               , i, base, mask, m_base.type, base, base+len, len);

         vmx_ept_map_mtrr(base, len, attr);
      }
   }
}

static void vmx_ept_map_with_mtrr()
{
   debug(VMX_EPT, "\n- Map EPT mem with MTRR\n");

   npg_map(0, info->hrd.mem.top, npg_dft_attr);
   _npg_remap_finest_4K(0);

   vmx_ept_map_mtrr_variable();

   if(info->vm.mtrr_def.fe && info->vm.mtrr_cap.fix)
      vmx_ept_map_mtrr_fixed();

   npg_unmap(info->area.start, info->area.end);
}

static void vmx_ept_map_no_mtrr()
{
   npg_map(0, info->area.start, npg_dft_attr);
   npg_map(info->area.end, info->hrd.mem.top, npg_dft_attr);
}

void vmx_ept_map()
{
   if(info->vm.mtrr_def.e)
      vmx_ept_map_with_mtrr();
   else
      vmx_ept_map_no_mtrr();
}

void vmx_ept_unmap()
{
   wbinvd();
   invept(VMCS_EPT_INV_ALL);
#ifdef CONFIG_VMX_FEAT_VPID
   invvpid(VMCS_VPID_INV_ALL);
#endif
   npg_unmap(0, info->hrd.mem.top);
}

void vmx_ept_remap()
{
   vmx_ept_unmap();
   vmx_ept_map();
}
