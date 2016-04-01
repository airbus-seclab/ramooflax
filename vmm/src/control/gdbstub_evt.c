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
#include <ctrl_evt.h>
#include <gdbstub.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int gdbstub_evt_cr_rd(arg_t arg)
{
   debug(GDBSTUB_EVT, "preempt(rd_cr %d)\n", arg.blow);
   gdb_preempt(GDB_EXIT_R_CR0+(uint8_t)arg.blow);
   return VM_DONE;
}

static int gdbstub_evt_cr_wr(arg_t arg)
{
   debug(GDBSTUB_EVT, "preempt(wr_cr %d)\n", arg.blow);
   gdb_preempt(GDB_EXIT_W_CR0+(uint8_t)arg.blow);
   return VM_DONE;
}

static int gdbstub_evt_excp(arg_t arg)
{
   debug(GDBSTUB_EVT, "preempt(excp %d)\n", arg.low);
   gdb_preempt(GDB_EXIT_EXCP_DE+(uint8_t)arg.low);
   return VM_DONE;
}

static int gdbstub_evt_trap(arg_t __unused__ arg)
{
   debug(GDBSTUB_EVT, "preempt(trap)\n");
   gdb_preempt(GDB_EXIT_TRAP);
   return VM_DONE;
}

static int gdbstub_evt_npf(arg_t __unused__ arg)
{
   debug(GDBSTUB_EVT, "preempt(npf)\n");
   gdb_preempt(GDB_EXIT_NPF);
   return VM_DONE;
}

static int gdbstub_evt_hyp(arg_t __unused__ arg)
{
   debug(GDBSTUB_EVT, "preempt(hyp)\n");
   gdb_preempt(GDB_EXIT_HYP);
   return VM_DONE;
}

static int gdbstub_evt_cpuid(arg_t __unused__ arg)
{
   debug(GDBSTUB_EVT, "preempt(cpuid)\n");
   gdb_preempt(GDB_EXIT_CPUID);
   return VM_DONE;
}

ctrl_evt_hdl_t ctrl_evt_usr_hdl[] = {
   gdbstub_evt_cr_rd,
   gdbstub_evt_cr_wr,
   gdbstub_evt_excp,
   gdbstub_evt_trap,
   gdbstub_evt_trap,
   gdbstub_evt_npf,
   gdbstub_evt_hyp,
   gdbstub_evt_cpuid,
};
