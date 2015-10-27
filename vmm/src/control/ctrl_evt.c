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
#include <ctrl.h>
#include <debug.h>
#include <disasm.h>
#include <emulate.h>
#include <emulate_insn.h>
#include <info_data.h>

extern info_data_t    *info;
extern ctrl_evt_hdl_t ctrl_evt_dft_hdl[];

int __ctrl_evt_excp_dbg(uint32_t vector)
{
   arg_t arg;

   if(!(info->vmm.ctrl.dbg.excp & (1<<vector)))
      return VM_IGNORE;

   debug(CTRL_EVT, "ctrl dbg excp\n");

   switch(vector)
   {
   case DB_EXCP: return dbg_evt_hard();
   case BP_EXCP: return dbg_evt_soft();
   case GP_EXCP: return dbg_evt_gp();
   }

   debug(CTRL_EVT, "unhandled ctrl dbg excp %d\n", vector);
   arg.low = vector;
   ctrl_evt_setup(CTRL_EVT_TYPE_EXCP, 0, arg);
   return VM_DONE;
}

static int __ctrl_evt_excp_user(uint32_t vector)
{
   if(info->vmm.ctrl.usr.excp & (1<<vector))
   {
      arg_t arg;
      arg.low = vector;
      ctrl_evt_setup(CTRL_EVT_TYPE_EXCP, 0, arg);
      debug(CTRL_EVT, "ctrl usr excp\n");
   }

   /* force vmm to inject exception */
   return VM_IGNORE;
}

static int __ctrl_evt_excp_vmm_np()
{
   debug(CTRL_EVT, "#NP\n");
   return VM_IGNORE;
}

static int __ctrl_evt_excp_vmm_gp()
{
   debug(CTRL_EVT, "#GP\n");
   return VM_IGNORE;
}

static int __ctrl_evt_excp_vmm(uint32_t vector)
{
   if(!(info->vm.cpu.dflt_excp & (1<<vector)))
      return VM_IGNORE;

   debug(CTRL_EVT, "ctrl vmm excp\n");

   switch(vector)
   {
   case NP_EXCP: return __ctrl_evt_excp_vmm_np();
   case GP_EXCP: return __ctrl_evt_excp_vmm_gp();
   }

   debug(CTRL_EVT, "unhandled ctrl vmm excp %d (ignoring)\n", vector);
   return VM_IGNORE;
}

int ctrl_evt_excp(uint32_t vector)
{
   int rc;

   if((rc = __ctrl_evt_excp_vmm(vector)) == VM_IGNORE)
      if((rc = __ctrl_evt_excp_dbg(vector)) == VM_IGNORE)
	 rc = __ctrl_evt_excp_user(vector);

   return rc;
}

int ctrl_evt_cr_rd(uint8_t n)
{
   if(info->vmm.ctrl.usr.cr_rd & (1<<n))
   {
      arg_t arg;
      arg.blow = n;
      ctrl_evt_setup(CTRL_EVT_TYPE_CR_RD, 0, arg);
      return VM_DONE;
   }

   return VM_IGNORE;
}

int ctrl_evt_cr_wr(uint8_t n)
{
   if(info->vmm.ctrl.usr.cr_wr & (1<<n))
   {
      arg_t arg;
      arg.blow = n;
      ctrl_evt_setup(CTRL_EVT_TYPE_CR_WR, 0, arg);
      return VM_DONE;
   }

   return VM_IGNORE;
}

int ctrl_evt_npf()
{
   if(!(info->vmm.ctrl.usr.filter & CTRL_FILTER_NPF))
      return VM_IGNORE;

   arg_t arg = {.raw = 0};
   ctrl_evt_setup(CTRL_EVT_TYPE_NPF, 0, arg);
   return VM_DONE;
}

int ctrl_evt_hypercall()
{
   if(!(info->vmm.ctrl.usr.filter & CTRL_FILTER_HYP))
      return VM_IGNORE;

   arg_t arg = {.raw = 0};
   ctrl_evt_setup(CTRL_EVT_TYPE_HYP, 0, arg);
   return VM_DONE;
}

int ctrl_evt_cpuid()
{
   if(!(info->vmm.ctrl.usr.filter & CTRL_FILTER_CPUID))
      return VM_IGNORE;

   arg_t arg = {.raw = 0};
   ctrl_evt_setup(CTRL_EVT_TYPE_CPUID, 0, arg);
   return VM_DONE;
}

int ctrl_evt_setup(uint8_t type, ctrl_evt_hdl_t hdl, arg_t arg)
{
   info->vmm.ctrl.event.type = type;
   info->vmm.ctrl.event.arg  = arg;

   if(hdl)
      info->vmm.ctrl.event.hdl = hdl;
   else
   {
      debug(CTRL_EVT, "using dft ctrl evt table 0x%X\n", ctrl_evt_dft_hdl);
      info->vmm.ctrl.event.hdl = ctrl_evt_dft_hdl[type];
   }

   debug(CTRL_EVT
   	 , "setup ctrl evt %d hdl 0x%X arg 0x%X\n"
   	 , info->vmm.ctrl.event.type
   	 , info->vmm.ctrl.event.hdl
   	 , info->vmm.ctrl.event.arg);
   return 1;
}

int ctrl_event()
{
   int rc;

   if(!info->vmm.ctrl.event.hdl)
      return VM_DONE;

   debug(CTRL_EVT, "calling ctrl evt handler 0x%X\n", info->vmm.ctrl.event.hdl);
   rc = info->vmm.ctrl.event.hdl(info->vmm.ctrl.event.arg);
   info->vmm.ctrl.event.hdl = 0;
   return rc;
}
