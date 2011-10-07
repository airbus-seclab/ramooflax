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
#include <gdb.h>
#include <insn.h>
#include <excp.h>
#include <debug.h>
#include <emulate.h>
#include <info_data.h>

extern info_data_t *info;

void gdb_singlestep_set_rflags(rflags_reg_t *rflags)
{
   info->vmm.ctrl.dbg.stp.status.tf = rflags->tf;
   rflags->tf = 1;
}

void gdb_singlestep_restore_rflags(rflags_reg_t *rflags)
{
   rflags->tf = info->vmm.ctrl.dbg.stp.status.tf;
}

int gdb_singlestep_correct_rflags(rflags_reg_t *rflags)
{
   return (rflags->tf)?1:0;
}

void gdb_singlestep_save_context()
{
   info->vmm.ctrl.dbg.stp.ctx.cr3.raw = __cr3.raw;

   if(!__pmode32())
      return;

   /* XXX: need syscall/sysret support too */
   __pre_access(__sysenter_cs);
   info->vmm.ctrl.dbg.stp.ctx.sysenter_cs.raw = __sysenter_cs.raw;

   debug(GDB_CMD,"save context sysenter_cs 0x%X\n", __sysenter_cs.raw);
   //__sysenter_cs.raw = 0;
}

void gdb_singlestep_restore_context()
{
   debug(GDB_CMD,"restore context sysenter_cs 0x%X\n",
	 info->vmm.ctrl.dbg.stp.ctx.sysenter_cs.raw);

   __sysenter_cs.raw = info->vmm.ctrl.dbg.stp.ctx.sysenter_cs.raw;
   __post_access(__sysenter_cs);
}

int gdb_singlestep_enable(uint8_t requestor)
{
   gdb_singlestep_set_requestor(requestor);

   if(info->vmm.ctrl.dbg.stp.status.on)
      return 1;

   info->vmm.ctrl.dbg.stp.status.on = 1;
   gdb_singlestep_save_context();

   gdb_singlestep_set_rflags(&__rflags);
   __post_access(__rflags);
   gdb_protect_db_excp();

   return 1;
}

void gdb_singlestep_disable()
{
   if(!info->vmm.ctrl.dbg.stp.status.on)
      return;

   info->vmm.ctrl.dbg.stp.status.on = 0;
   gdb_singlestep_restore_context();

   gdb_singlestep_restore_rflags(&__rflags);
   __post_access(__rflags);
   gdb_release_db_excp();
}

int gdb_singlestep_check()
{
   if(!gdb_enabled() || !gdb_singlestep_enabled())
      return 0;

   return 1;
}

int __gdb_singlestep_fake()
{
   int rc = gdb_singlestep();

   if(rc == GDB_PREEMPT)
   {
      __dr6.bs = 1;
      __set_accessed(__dr6);
   }

   return rc;
}

int gdb_singlestep_fake()
{
   if(__gdb_singlestep_fake() == GDB_PREEMPT)
      return gdb_preempt(GDB_EXIT_TRAP);

   return 0;
}

int gdb_singlestep()
{
   if(!gdb_singlestep_check())
      return GDB_IGNORE;

   debug(GDB_CMD, "singlestep event\n");

   gdb_brk_hrd_set_last(0, DR7_COND_SS);
   gdb_dr6_set_dirty(1);
   gdb_singlestep_disable();

   if(gdb_brk_mem_need_restore())
   {
      gdb_brk_mem_restore_bp();
      gdb_brk_mem_set_restore(0);
   }

   if(gdb_singlestep_requestor() == GDB_SSTEP_VMM)
   {
      debug(GDB_CMD, "singlestep for VMM ... release\n");
      return GDB_RELEASE;
   }

   debug(GDB_CMD, "singlestep for USER ... preempt\n");
   return GDB_PREEMPT;
}
