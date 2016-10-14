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
#include <ctrl.h>
#include <debug.h>
#include <disasm.h>
#include <emulate.h>
#include <emulate_insn.h>
#include <info_data.h>

extern info_data_t    *info;
extern ctrl_evt_hdl_t ctrl_evt_usr_hdl[];
extern ctrl_evt_hdl_t ctrl_evt_vmm_hdl[];

/*
** Exceptions
*/
static int __ctrl_evt_excp_vmm(uint32_t vector)
{
   if(info->vm.cpu.dflt_excp & (1<<vector))
   {
      arg_t arg;
      arg.low = vector;
      debug(CTRL_EVT, "ctrl vmm excp\n");

      return ctrl_evt_vmm_hdl[CTRL_EVT_VMM_TYPE_EXCP](arg);
   }

   debug(CTRL_EVT, "unhandled ctrl vmm excp %d (ignoring)\n", vector);
   return VM_IGNORE;
}

static int __ctrl_evt_excp_dbg(uint32_t vector)
{
   if(info->vmm.ctrl.dbg.excp & (1<<vector))
   {
      debug(CTRL_EVT, "ctrl dbg excp\n");

      switch(vector)
      {
      case DB_EXCP: return dbg_evt_hard();
      case BP_EXCP: return dbg_evt_soft();
      case GP_EXCP: return dbg_evt_gp();
      }
   }

   debug(CTRL_EVT, "unhandled ctrl dbg excp %d (ignoring)\n", vector);
   return VM_IGNORE;
}

static int __ctrl_evt_excp_usr(uint32_t vector)
{
   if(info->vmm.ctrl.usr.excp & (1<<vector))
   {
      arg_t arg;
      arg.low = vector;
      ctrl_evt_setup(CTRL_EVT_USR_TYPE_EXCP, 0, arg);
      debug(CTRL_EVT, "ctrl usr excp\n");
   }

   /* force vmm to inject exception */
   return VM_IGNORE;
}

int ctrl_evt_excp(uint32_t vector)
{
   int rc;

   if((rc = __ctrl_evt_excp_vmm(vector)) == VM_IGNORE)
      if((rc = __ctrl_evt_excp_dbg(vector)) == VM_IGNORE)
         rc = __ctrl_evt_excp_usr(vector);

   return rc;
}

/*
** Control Registers
*/
static int __ctrl_evt_cr_wr_vmm(uint8_t n)
{
   arg_t arg;
   arg.blow = n;
   return ctrl_evt_vmm_hdl[CTRL_EVT_VMM_TYPE_WCR](arg);
}

static int __ctrl_evt_cr_wr_usr(uint8_t n)
{
   if(!(info->vmm.ctrl.usr.cr_wr & (1<<n)))
      return VM_IGNORE;

   arg_t arg;
   arg.blow = n;
   ctrl_evt_setup(CTRL_EVT_USR_TYPE_CR_WR, 0, arg);
   return VM_DONE;
}

int ctrl_evt_cr_rd(uint8_t n)
{
   if(info->vmm.ctrl.usr.cr_rd & (1<<n))
   {
      arg_t arg;
      arg.blow = n;
      ctrl_evt_setup(CTRL_EVT_USR_TYPE_CR_RD, 0, arg);
      return VM_DONE;
   }

   return VM_IGNORE;
}

int ctrl_evt_cr_wr(uint8_t n)
{
   int rc;

   if((rc = __ctrl_evt_cr_wr_vmm(n)) == VM_IGNORE)
      rc = __ctrl_evt_cr_wr_usr(n);

   return rc;
}

/*
** Nested Page Faults
*/
static int __ctrl_evt_npf_vmm()
{
   arg_t arg = {.raw = 0};
   return ctrl_evt_vmm_hdl[CTRL_EVT_VMM_TYPE_NPF](arg);
}

static int __ctrl_evt_npf_usr()
{
   if(!(info->vmm.ctrl.usr.filter & CTRL_FILTER_NPF))
      return VM_IGNORE;

   arg_t arg = {.raw = 0};
   ctrl_evt_setup(CTRL_EVT_USR_TYPE_NPF, 0, arg);
   return VM_DONE;
}

int ctrl_evt_npf()
{
   int rc;

   if((rc = __ctrl_evt_npf_vmm()) == VM_IGNORE)
      rc = __ctrl_evt_npf_usr();

   return rc;
}

/*
** ACPI S3 Sleep Mode
*/
static int __ctrl_evt_suspend_vmm()
{
   arg_t arg = {.raw = 0};
   return ctrl_evt_vmm_hdl[CTRL_EVT_VMM_TYPE_S3](arg);
}

static int __ctrl_evt_suspend_usr()
{
   if(!(info->vmm.ctrl.usr.filter & CTRL_FILTER_S3))
      return VM_IGNORE;

   arg_t arg = {.raw = 0};
   ctrl_evt_setup(CTRL_EVT_USR_TYPE_S3, 0, arg);
   return VM_DONE;
}

int ctrl_evt_suspend()
{
   int rc;

   if((rc = __ctrl_evt_suspend_vmm()) == VM_IGNORE)
      rc = __ctrl_evt_suspend_usr();

   return rc;
}


/*
** Hypercall
*/
static int __ctrl_evt_hyp_vmm()
{
   arg_t arg = {.raw = 0};
   return ctrl_evt_vmm_hdl[CTRL_EVT_VMM_TYPE_HYP](arg);
}

static int __ctrl_evt_hyp_usr()
{
   if(!(info->vmm.ctrl.usr.filter & CTRL_FILTER_HYP))
      return VM_IGNORE;

   arg_t arg = {.raw = 0};
   ctrl_evt_setup(CTRL_EVT_USR_TYPE_HYP, 0, arg);
   return VM_DONE;
}

int ctrl_evt_hypercall()
{
   int rc;

   if((rc = __ctrl_evt_hyp_vmm()) == VM_IGNORE)
      rc = __ctrl_evt_hyp_usr();

   return rc;
}

/*
** CPUID
*/
int ctrl_evt_cpuid(uint32_t index)
{
   if(!(info->vmm.ctrl.usr.filter & CTRL_FILTER_CPUID))
      return VM_IGNORE;

   if(info->vmm.ctrl.usr.cpuid != index &&
      info->vmm.ctrl.usr.cpuid != CTRL_CPUID_ALL)
      return VM_IGNORE;

   arg_t arg = {.raw = 0};
   ctrl_evt_setup(CTRL_EVT_USR_TYPE_CPUID, 0, arg);
   return VM_DONE;
}

/*
** Event Services
*/
int ctrl_evt_setup(uint8_t type, ctrl_evt_hdl_t hdl, arg_t arg)
{
   info->vmm.ctrl.event.type = type;
   info->vmm.ctrl.event.arg  = arg;

   if(hdl)
      info->vmm.ctrl.event.hdl = hdl;
   else
   {
      debug(CTRL_EVT, "using usr ctrl evt table 0x%X\n", ctrl_evt_usr_hdl);
      info->vmm.ctrl.event.hdl = ctrl_evt_usr_hdl[type];
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
