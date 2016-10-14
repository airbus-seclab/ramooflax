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
#include <vmx_exit_invg.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

#define WRN " !** oO0Oo **! "

static void vmx_invg_cr_dr_msr()
{
   bool_t  uguest;
   raw64_t x;

   x.raw = 0;
   x.low = vm_state.cr0.low;
   vmx_set_fixed(x.low, info->vm.vmx_fx_cr0);

   if(x.low != vm_state.cr0.low)
      printf(WRN"cr0 setting\n"
             "cr0         = %b\n"
             "cr0 fixed 1 = %b\n"
             "cr0 allow 1 = %b\n"
             ,vm_state.cr0.low
             ,info->vm.vmx_fx_cr0.fixed_1.raw
             ,info->vm.vmx_fx_cr0.allow_1.raw);

   uguest = vm_exec_ctrls.proc.proc2 & vm_exec_ctrls.proc2.uguest;
   if(!uguest && !(vm_state.cr0.pe & vm_state.cr0.pg))
      printf(WRN"cr0 setting (uguest %d pe %d pg %d\n"
             ,uguest, vm_state.cr0.pe, vm_state.cr0.pg);

   x.raw = 0;
   x.low = vm_state.cr4.low;
   vmx_set_fixed(x.low, info->vm.vmx_fx_cr4);

   if(x.low != vm_state.cr4.low)
      printf(WRN"cr4 setting\n"
             "cr4         = %b\n"
             "cr4 fixed 1 = %b\n"
             "cr4 allow 1 = %b\n"
             ,vm_state.cr4.low
             ,info->vm.vmx_fx_cr4.fixed_1.raw
             ,info->vm.vmx_fx_cr4.allow_1.raw);

   if(vm_entry_ctrls.entry.load_dbgctl)
   {
      if(vm_state.ia32_dbgctl.r1 ||
         vm_state.ia32_dbgctl.r2 ||
         vm_state.ia32_dbgctl.r3)
      {
         printf(WRN"ia32 dbgctl rsv setting\n");
         goto __log_dbgctl;
      }

      if(vm_state.dr7.high)
      {
         printf(WRN"ia32 dbgctl (dr7)\n");

      __log_dbgctl:
         printf("ia32 dbg ctl rsv bits   : %d %d %D\n"
                "ia32 dbg ctl dr7 [63-32]: 0x%x\n"
                ,vm_state.ia32_dbgctl.r1, vm_state.ia32_dbgctl.r2
                ,vm_state.ia32_dbgctl.r3, vm_state.dr7.high);
      }
   }

   if(vm_entry_ctrls.entry.ia32e && !(vm_state.cr0.pg & vm_state.cr4.pae))
      printf(WRN"ia32e mode (pg/pae)\n");

   if(!vm_entry_ctrls.entry.ia32e && vm_state.cr4.pcide)
      printf(WRN"ia32e mode (pcide)\n");

   if(vm_state.cr3.raw & ~info->vm.cpu.max_paddr)
      printf(WRN"cr3 setting\n");

   if(!canonical_linear(vm_state.ia32_sysenter_esp.raw))
   {
      printf(WRN"sysenter esp\n");
      goto __log_sysenter;
   }

   if(!canonical_linear(vm_state.ia32_sysenter_eip.raw))
   {
      printf(WRN"sysenter eip\n");

   __log_sysenter:
      printf("sysenter esp = 0x%X eip = 0x%X\n"
             ,vm_state.ia32_sysenter_esp.raw
             ,vm_state.ia32_sysenter_eip.raw);
   }

   if(vm_entry_ctrls.entry.load_ia32_perf &&
      (vm_state.ia32_perf.r1 || vm_state.ia32_perf.r2))
      printf(WRN"ia32 perf rsv setting\n"
             "ia32 perf rsv bits : %d %d %D\n"
             ,vm_state.ia32_perf.r1, vm_state.ia32_perf.r2);

   if(vm_entry_ctrls.entry.load_ia32_pat &&
      ((vm_state.ia32_pat.pa0 == 2 || vm_state.ia32_pat.pa0 == 3) ||
       (vm_state.ia32_pat.pa1 == 2 || vm_state.ia32_pat.pa1 == 3) ||
       (vm_state.ia32_pat.pa2 == 2 || vm_state.ia32_pat.pa2 == 3) ||
       (vm_state.ia32_pat.pa3 == 2 || vm_state.ia32_pat.pa3 == 3) ||
       (vm_state.ia32_pat.pa4 == 2 || vm_state.ia32_pat.pa4 == 3) ||
       (vm_state.ia32_pat.pa5 == 2 || vm_state.ia32_pat.pa5 == 3) ||
       (vm_state.ia32_pat.pa6 == 2 || vm_state.ia32_pat.pa6 == 3) ||
       (vm_state.ia32_pat.pa7 == 2 || vm_state.ia32_pat.pa7 == 3)))
      printf(WRN"ia32 pat setting 0x%X\n", vm_state.ia32_pat);

   if(vm_entry_ctrls.entry.load_ia32_efer)
   {
      if(vm_state.ia32_efer.raw & (~0xd01ULL))
         printf(WRN"ia32 efer rsv bits setting\n");

      if(vm_state.ia32_efer.lma != vm_entry_ctrls.entry.ia32e)
         printf(WRN"efer lma setting\n");

      if((!uguest && !vm_state.cr0.pg) &&
         (vm_state.ia32_efer.lma != vm_state.ia32_efer.lme))
         printf(WRN"efer lme setting\n");
   }
}

static void vmx_check_seg_limit(char *nm, vmcs_guest_seg_desc_t *seg)
{
   if(!seg->attributes.g && (seg->limit.raw>>20))
      printf(WRN"%s granularity/limit\n", nm);
}

static void vmx_check_seg_type(char *nm, vmcs_guest_seg_desc_t *seg, bool_t uguest)
{
   if(seg->attributes.u)
      return;

   if(!uguest && seg->attributes.type<11 && seg->attributes.dpl < seg->selector.rpl)
      printf(WRN"%s dpl\n", nm);

   if(!(seg->attributes.type&(1<<0)))
      printf(WRN"%s not accessed\n", nm);

   if((seg->attributes.type&(1<<3)) && !(seg->attributes.type&(1<<1)))
      printf(WRN"%s type (code/unreadable)\n", nm);

   if(!seg->attributes.s)
      printf(WRN"%s system type\n", nm);

   if(!seg->attributes.p)
      printf(WRN"%s not present\n", nm);

   if(seg->attributes.r1 || seg->attributes.r2)
      printf(WRN"%s rsv bits\n", nm);

   vmx_check_seg_limit(nm, seg);
}

static void vmx_invg_segments()
{
   bool_t uguest = vm_exec_ctrls.proc.proc2 & vm_exec_ctrls.proc2.uguest;

   if(vm_state.tr.selector.ti)
      printf(WRN"TR not in GDT\n");

   if(!vm_state.ldtr.attributes.u && vm_state.ldtr.selector.ti)
      printf(WRN"LDTR usable and not in GDT\n");

   if(!vm_state.rflags.vm && !uguest &&
      vm_state.ss.selector.rpl != vm_state.cs.selector.rpl)
      printf(WRN"ss/cs RPL\n");

   if(vm_state.rflags.vm)
   {
      if((vm_state.cs.base.raw != (uint64_t)vm_state.cs.selector.index*16) ||
         (vm_state.ss.base.raw != (uint64_t)vm_state.ss.selector.index*16) ||
         (vm_state.ds.base.raw != (uint64_t)vm_state.ds.selector.index*16) ||
         (vm_state.es.base.raw != (uint64_t)vm_state.es.selector.index*16) ||
         (vm_state.fs.base.raw != (uint64_t)vm_state.fs.selector.index*16) ||
         (vm_state.gs.base.raw != (uint64_t)vm_state.gs.selector.index*16))
         printf(WRN"v86 segment base\n");

      if(vm_state.cs.limit.raw != 0xffff ||
         vm_state.cs.limit.raw != 0xffff ||
         vm_state.cs.limit.raw != 0xffff ||
         vm_state.cs.limit.raw != 0xffff ||
         vm_state.cs.limit.raw != 0xffff ||
         vm_state.cs.limit.raw != 0xffff)
         printf(WRN"v86 segment limit\n");

      if(vm_state.cs.attributes.raw != 0xf3 ||
         vm_state.ss.attributes.raw != 0xf3 ||
         vm_state.ds.attributes.raw != 0xf3 ||
         vm_state.es.attributes.raw != 0xf3 ||
         vm_state.fs.attributes.raw != 0xf3 ||
         vm_state.gs.attributes.raw != 0xf3)
         printf(WRN"v86 segment attr\n");
   }

   if(!canonical_linear(vm_state.tr.base.raw) ||
      !canonical_linear(vm_state.fs.base.raw) ||
      !canonical_linear(vm_state.gs.base.raw))
      printf(WRN"TR/FS/GS non-canonical base address\n");

   if(!vm_state.ldtr.attributes.u && !canonical_linear(vm_state.ldtr.base.raw))
      printf(WRN"LDTR non-canonical base address\n");

   if(vm_state.cs.base.high != 0)
      printf(WRN"CS base address upper bits non zero\n");

   if(!vm_state.ss.attributes.u && vm_state.ss.base.high)
      printf(WRN"SS invalid base address\n");

   if(!vm_state.ds.attributes.u && vm_state.ds.base.high)
      printf(WRN"DS invalid base address\n");

   if(!vm_state.es.attributes.u && vm_state.es.base.high)
      printf(WRN"ES invalid base address\n");

   if(!vm_state.rflags.vm)
   {
      if(!(vm_state.cs.attributes.type == 9  ||
           vm_state.cs.attributes.type == 11 ||
           vm_state.cs.attributes.type == 13 ||
           vm_state.cs.attributes.type == 15))
      {
         if(!uguest)
            printf(WRN"CS type\n");
         else if(vm_state.cs.attributes.type != 3)
            printf(WRN"CS type while uguest\n");
      }

      if(!vm_state.cs.attributes.s)
         printf(WRN"CS desc (sys)\n");

      if(!vm_state.cs.attributes.p)
         printf(WRN"CS present bit\n");

      if(vm_state.cs.attributes.r1 || vm_state.cs.attributes.r2)
         printf(WRN"CS reserved bits\n");

      if(vm_entry_ctrls.entry.ia32e &&
         vm_state.cs.attributes.l &&
         vm_state.cs.attributes.d)
         printf(WRN"CS.attr.d while entering ia32e\n");

      vmx_check_seg_limit("CS", &vm_state.cs);

      if(vm_state.cs.attributes.type == 3 &&
         (vm_state.cs.attributes.dpl || vm_state.ss.attributes.dpl))
         printf(WRN"CS/SS dpl (type 3)\n");

      if((vm_state.cs.attributes.type == 9 || vm_state.cs.attributes.type == 11) &&
         vm_state.cs.attributes.dpl != vm_state.ss.attributes.dpl)
         printf(WRN"CS dpl (type 9/11)\n");

      if((vm_state.cs.attributes.type == 13 || vm_state.cs.attributes.type == 15) &&
         vm_state.cs.attributes.dpl > vm_state.ss.attributes.dpl)
         printf(WRN"CS dpl (type 13/15)\n");

      if(!vm_state.ss.attributes.u &&
         (!vm_state.ss.attributes.s ||
          !(vm_state.ss.attributes.type == 3 || vm_state.ss.attributes.type == 7)))
         printf(WRN"SS type/sys\n");

      if(!uguest && vm_state.ss.attributes.dpl != vm_state.ss.selector.rpl)
         printf(WRN"SS dpl\n");

      if(uguest && !vm_state.cr0.pe && vm_state.ss.attributes.dpl)
         printf(WRN"SS dpl (pe=0)\n");

      if(!vm_state.ss.attributes.u)
      {
         if(!vm_state.ss.attributes.p)
            printf(WRN"SS present bit\n");

         if(vm_state.ss.attributes.r1 || vm_state.ss.attributes.r2)
            printf(WRN"SS rsv bits\n");

         vmx_check_seg_limit("SS", &vm_state.ss);
      }

      vmx_check_seg_type("DS", &vm_state.ds, uguest);
      vmx_check_seg_type("ES", &vm_state.es, uguest);
      vmx_check_seg_type("FS", &vm_state.fs, uguest);
      vmx_check_seg_type("GS", &vm_state.gs, uguest);
   }

   if((vm_entry_ctrls.entry.ia32e  &&
       vm_state.tr.attributes.type != 11) ||
      (!vm_entry_ctrls.entry.ia32e &&
       vm_state.tr.attributes.type != 3 &&
       vm_state.tr.attributes.type != 11))
      printf(WRN"TR type\n");

   if(vm_state.tr.attributes.s)
      printf(WRN"TR sys\n");

   if(!vm_state.tr.attributes.p)
      printf(WRN"TR not present\n");

   if(vm_state.tr.attributes.r1 || vm_state.tr.attributes.r2)
      printf(WRN"TR rsv bits\n");

   vmx_check_seg_limit("TR", &vm_state.tr);

   if(vm_state.tr.attributes.u)
      printf(WRN"TR unusable\n");

   if(!vm_state.ldtr.attributes.u)
   {
      if(vm_state.ldtr.attributes.type != 2)
         printf(WRN"LDTR type\n");

      if(vm_state.ldtr.attributes.s)
         printf(WRN"LDTR sys\n");

      if(!vm_state.ldtr.attributes.p)
         printf(WRN"LDTR not present\n");

      if(vm_state.ldtr.attributes.r1 || vm_state.ldtr.attributes.r2)
         printf(WRN"LDTR rsv bits\n");

      vmx_check_seg_limit("LDTR", &vm_state.ldtr);
   }
}

static void vmx_invg_dtr()
{
   if(!canonical_linear(vm_state.gdtr.base.raw))
      printf(WRN"GDT non-canonical base address\n");

   if(!canonical_linear(vm_state.idtr.base.raw))
      printf(WRN"IDT non-canonical base address\n");

   if(vm_state.gdtr.limit.whigh)
      printf(WRN"GDTR limit\n");

   if(vm_state.idtr.limit.whigh)
      printf(WRN"IDTR limit\n");
}

static void vmx_invg_loc()
{
   if(vm_entry_ctrls.entry.ia32e && vm_state.cs.attributes.l)
   {
      if(!canonical_linear(vm_state.rip.raw))
         printf(WRN"RIP non-canonical address\n");
   }
   else if(vm_state.rip.high)
      printf(WRN"RIP upper bits\n");

   if(!vm_state.rflags.r1 ||
      (vm_state.rflags.r2 || vm_state.rflags.r3 ||
       vm_state.rflags.r4 || vm_state.rflags.r5))
      printf(WRN"RFLAGS rsv bits\n");

   if(vm_state.rflags.vm && (vm_entry_ctrls.entry.ia32e || !vm_state.cr0.pe))
      printf(WRN"RFLAGS vm bit\n");

   if(!vm_state.rflags.IF &&
      (vm_entry_ctrls.int_info.v &&
       vm_entry_ctrls.int_info.type == VMCS_EVT_INFO_TYPE_HW_INT))
      printf(WRN"RFLAGS.if = 0 while injecting IRQ\n");
}

static void vmx_invg_nonreg()
{
   if(vm_state.activity.raw > 3)
      printf(WRN"activity state\n");

   if(vm_state.activity.raw == VMX_VMCS_GUEST_ACTIVITY_STATE_HALT &&
       vm_state.ss.attributes.dpl)
      printf("activity state (HLT) ss.dpl\n");

   if(vm_state.activity.raw != VMX_VMCS_GUEST_ACTIVITY_STATE_ACTIVE &&
      (vm_state.interrupt.sti || vm_state.interrupt.mss))
      printf("activity state (ACTIVE) sti/mss\n");

   if(vm_entry_ctrls.int_info.v)
   {
      if(vm_state.activity.raw == VMX_VMCS_GUEST_ACTIVITY_STATE_HALT &&
         (
            !(vm_entry_ctrls.int_info.type == VMCS_EVT_INFO_TYPE_HW_INT ||
              vm_entry_ctrls.int_info.type == VMCS_EVT_INFO_TYPE_NMI)
            ||
            !(vm_entry_ctrls.int_info.type == VMCS_EVT_INFO_TYPE_HW_EXCP &&
              (vm_entry_ctrls.int_info.vector == DB_EXCP ||
               vm_entry_ctrls.int_info.vector == MC_EXCP))
            ||
            !(vm_entry_ctrls.int_info.type == VMCS_EVT_INFO_TYPE_OTHER &&
              vm_entry_ctrls.int_info.vector == 0)
            ))
         printf(WRN"activity state (HLT) event entry\n");
      else if(vm_state.activity.raw == VMX_VMCS_GUEST_ACTIVITY_STATE_SHUTDOWN &&
              !(vm_entry_ctrls.int_info.type == VMCS_EVT_INFO_TYPE_HW_EXCP &&
                (vm_entry_ctrls.int_info.vector == NMI_EXCP ||
                 vm_entry_ctrls.int_info.vector == MC_EXCP)))
         printf(WRN"activity state (SHTDWN)\n");
      else if(vm_state.activity.raw == VMX_VMCS_GUEST_ACTIVITY_STATE_SIPI)
         printf(WRN"activity state (SIPI) event entry\n");
   }

   if(vm_state.activity.raw == VMX_VMCS_GUEST_ACTIVITY_STATE_SIPI &&
      vm_entry_ctrls.entry.smm)
      printf(WRN"activity state (SIPI) smm\n");

   if(vm_state.interrupt.rsv)
      printf(WRN"interruptibility state rsv bits\n");

   if(vm_state.interrupt.sti && vm_state.interrupt.mss)
      printf(WRN"interruptibility state sti & mss\n");

   if(vm_state.interrupt.sti && !vm_state.rflags.IF)
      printf(WRN"interruptibility state sti & !rflags.if\n");

   if((vm_state.interrupt.sti || vm_state.interrupt.mss) &&
      (vm_entry_ctrls.int_info.v &&
       vm_entry_ctrls.int_info.type == VMCS_EVT_INFO_TYPE_HW_INT))
      printf(WRN"interruptibility state (sti/mss & inject HWINT)\n");

   /* sti "maybe required" */
   if((vm_state.interrupt.sti || vm_state.interrupt.mss) &&
      (vm_entry_ctrls.int_info.v &&
       vm_entry_ctrls.int_info.type == VMCS_EVT_INFO_TYPE_NMI))
      printf(WRN"interruptibility state (sti/mss & inject NMI)\n");

   if(vm_state.interrupt.smi) /* we are not in SMM */
      printf(WRN"interruptibility state (smi & !SMM)\n");

   if(!vm_state.interrupt.smi && vm_entry_ctrls.entry.smm)
      printf(WRN"interruptibility state (enter smm & !smi)\n");

   if(vm_state.interrupt.nmi && vm_exec_ctrls.pin.vnmi && vm_entry_ctrls.int_info.v
      && vm_entry_ctrls.int_info.type == VMCS_EVT_INFO_TYPE_NMI)
      printf(WRN"interruptibility state (nmi & vnmi)\n");

   if(vm_state.dbg_excp.r1 || vm_state.dbg_excp.r2 || vm_state.dbg_excp.r3)
      printf(WRN"pending #DB rsv bits\n");

   if(vm_state.interrupt.sti || vm_state.interrupt.mss ||
      vm_state.activity.raw == VMX_VMCS_GUEST_ACTIVITY_STATE_HALT)
   {
      if(((vm_state.rflags.tf && !vm_state.ia32_dbgctl.btf) && !vm_state.dbg_excp.bs)
         ||
         ((!vm_state.rflags.tf || vm_state.ia32_dbgctl.btf) && vm_state.dbg_excp.bs))
         printf(WRN"pending #DB bs/tf/btf\n");
   }

   if(vm_state.vmcs_link_ptr.raw != -1ULL)
   {
      raw64_t current;
      raw32_t x;
      loc_t   l = {.linear = vm_state.vmcs_link_ptr.raw};

      if(vm_state.vmcs_link_ptr.wlow & 0xfff)
         printf(WRN"vmcs link ptr\n");

      if(vm_state.vmcs_link_ptr.raw > info->vmm.cpu.max_paddr)
         printf(WRN"vmcs link ptr (max paddr)\n");

      x.raw = *l.u32;
      printf(WRN"vmcs link ptr [0x%X] = 0x%x\n"
             ,vm_state.vmcs_link_ptr.raw, x.raw);

      if((x.raw & 0x7fffffff) != info->vm.vmx_info.revision_id)
         printf(WRN"vmcs link ptr revision id\n");

      if((x.raw>>31) != vm_exec_ctrls.proc2.vmcs_shdw)
         printf(WRN"vmcs link ptr shadow\n");

      current.raw = (offset_t)&info->vm.cpu.vmc->vm_cpu_vmcs;
      if(vm_entry_ctrls.entry.smm && vm_state.vmcs_link_ptr.raw == current.raw)
         printf(WRN"vmcs link ptr SMM/current\n");
   }
}

static void vmx_check_pdpe(pdpe_t *pdpe, int n)
{
   if(!valid_pae_pdpe(pdpe))
      printf(WRN"pae pdpe[%d] = 0x%X\n", n, pdpe->raw);
}

static void vmx_invg_pdpte()
{
   if(!__paged_pae())
      return;

   if(!vm_exec_ctrls.proc2.ept)
   {
      pdpe_t *pdp = (pdpe_t*)pg_32B_addr((offset_t)__cr3.pae.addr);

      printf("checking guest pdpe from cr3\n");
      vmx_check_pdpe(&pdp[0], 0);
      vmx_check_pdpe(&pdp[1], 1);
      vmx_check_pdpe(&pdp[2], 2);
      vmx_check_pdpe(&pdp[3], 3);
   }
   else
   {
      printf("checking guest pdpe from vmcs\n");
      vmx_check_pdpe(&vm_state.pdpe_0, 0);
      vmx_check_pdpe(&vm_state.pdpe_1, 1);
      vmx_check_pdpe(&vm_state.pdpe_2, 2);
      vmx_check_pdpe(&vm_state.pdpe_3, 3);
   }
}


static void vmx_invg_detect()
{
   vmx_invg_cr_dr_msr();
   vmx_invg_segments();
   vmx_invg_dtr();
   vmx_invg_loc();
   vmx_invg_nonreg();
   vmx_invg_pdpte();
}

void vmx_vmexit_show_invalid_guest()
{
   printf("\n- invalid guest state inspection: ");
   switch(vm_exit_info.qualification.low)
   {
   case 2 :printf("PDPTEs load failure\n");      break;
   case 3 :printf("NMI injection failure\n");    break;
   case 4 :printf("VMCS link pointer\n");break;
   default:printf("%d\n", vm_exit_info.qualification.low);
   }

   vmx_invg_detect();
}
