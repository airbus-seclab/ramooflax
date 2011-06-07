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
#include <info_data.h>

extern info_data_t *info;

/*
** Memory breakpoints (0xcc)
*/
void gdb_brk_mem_set(offset_t addr)
{
   size_t                 i;
   vmm_ctrl_dbg_brk_mem_t *brk;

   brk = &info->vmm.ctrl.dbg.brk.mem;
   for(i=0 ; i<GDB_DFLT_BRK_NR ; i++)
   {
      if(!brk->list[i].setup.on)
      {
	 brk->list[i].addr   = addr;
	 brk->list[i].setup.on = 1;

	 if(!gdb_read_mem(addr, &brk->list[i].byte, 1))
	 {
	    debug(GDB, "can not read mem to set breakpoint @ 0x%X\n", addr);
	    goto __access;
	 }

	 /* if(!gdb_write_mem(addr, &brk_insn, 1)) */
	 /* { */
	 /*    debug(GDB, "can not write mem to set breakpoint @ 0x%X\n", addr); */
	 /*    goto __access; */
	 /* } */

	 debug(GDB_CMD, "added mem breakpoint @ 0x%X\n", addr);
	 goto __success;
      }
      else if(brk->list[i].addr == addr && brk->list[i].setup.on)
      {
	 debug(GDB_CMD, "already mem breakpoint @ 0x%X\n", addr);
	 goto __success;
      }
   }

   debug(GDB_CMD, "no more mem breakpoint slots\n");
   gdb_err_oom();
   return;

__access:
   gdb_err_mem();
   return;

__success:
   gdb_protect_bp_excp();
   gdb_ok();
   return;
}

void gdb_brk_mem_del(offset_t addr)
{
   uint8_t                more;
   size_t                 i;
   vmm_ctrl_dbg_brk_mem_t *brk;

   more = 0;
   brk  = &info->vmm.ctrl.dbg.brk.mem;
   for(i=0 ; i<GDB_DFLT_BRK_NR ; i++)
   {
      if(brk->list[i].setup.on)
      {
	 if(brk->list[i].addr == addr)
	 {
	    brk->list[i].setup.on = 0;
	    if(!gdb_write_mem(addr, &brk->list[i].byte, 1))
	    {
	       debug(GDB, "can not write 0x%x to remove breakpoint @ 0x%X\n"
		     , brk->list[i].byte, addr);
	       gdb_err_mem();
	       return;
	    }

	    debug(GDB_CMD, "removed mem breakpoint @ 0x%X\n", addr);
	 }
	 else if(!more)
	    more = 1;
      }
   }

   if(!more)
      gdb_release_bp_excp();

   /* silently drop not found breakpoint */
   gdb_ok();
}

/*
** XXX: svm #bp exits on 0xcc, vmx may need vm_rewind_rip(1);
*/
int __gdb_brk_mem_restore_insn(vmm_ctrl_dbg_brk_mem_e_t *bp)
{
   if(!gdb_write_mem(bp->addr, &bp->byte, 1))
   {
      debug(GDB, "can not write mem to restore insn @ 0x%X\n", bp->addr);
      gdb_err_mem();
      return GDB_BP_FAIL;
   }

   debug(GDB_CMD, "restored insn @ 0x%X\n", bp->addr);
   return GDB_BP_OK;
}

int gdb_brk_mem_restore_insn()
{
   int                       rc;
   vmm_ctrl_dbg_brk_mem_e_t *bp = info->vmm.ctrl.dbg.brk.mem.last;

   /* already restored */
   if(!bp->setup.on || bp->addr != info->vmm.ctrl.dbg.brk.mem.last_addr)
      return GDB_BP_IGNORED;

   rc = __gdb_brk_mem_restore_insn(bp);
   if(rc == GDB_BP_OK)
   {
      debug(GDB_CMD, "marked bp to be restored\n");
      gdb_brk_mem_set_restore(1);
   }

   return rc;
}

int __gdb_brk_mem_restore_bp(vmm_ctrl_dbg_brk_mem_e_t *bp)
{
   uint8_t brk_insn = BREAKPOINT_INSN;

   if(!gdb_write_mem(bp->addr, &brk_insn, 1))
   {
      debug(GDB, "can not write mem to restore breakpoint @ 0x%X\n", bp->addr);
      return GDB_BP_FAIL;
   }

   debug(GDB_CMD, "restored mem breakpoint @ 0x%X\n", bp->addr);
   return GDB_BP_OK;
}

int gdb_brk_mem_restore_bp()
{
   return __gdb_brk_mem_restore_bp(info->vmm.ctrl.dbg.brk.mem.last);
}

void __gdb_brk_mem_disarm()
{
   size_t                 i;
   vmm_ctrl_dbg_brk_mem_t *brk = &info->vmm.ctrl.dbg.brk.mem;

   for(i=0 ; i<GDB_DFLT_BRK_NR ; i++)
      if(brk->list[i].setup.on)
	 __gdb_brk_mem_restore_insn(&brk->list[i]);

   info->vmm.ctrl.dbg.brk.mem.status.dis = 1;
}

void __gdb_brk_mem_rearm()
{
   size_t                 i;
   vmm_ctrl_dbg_brk_mem_t *brk = &info->vmm.ctrl.dbg.brk.mem;

   for(i=0 ; i<GDB_DFLT_BRK_NR ; i++)
      if(brk->list[i].setup.on)
      {
	 /* last bp hit must be singlestepped to resume insn */
	 if(gdb_brk_mem_need_restore() &&
	    brk->list[i].addr == info->vmm.ctrl.dbg.brk.mem.last_addr)
	    continue;

	 __gdb_brk_mem_restore_bp(&brk->list[i]);
      }

   info->vmm.ctrl.dbg.brk.mem.status.dis = 0;
}

int gdb_brk_mem_lookup(offset_t addr)
{
   size_t                 i;
   vmm_ctrl_dbg_brk_mem_t *brk = &info->vmm.ctrl.dbg.brk.mem;

   for(i=0 ; i<GDB_DFLT_BRK_NR ; i++)
   {
      if(brk->list[i].setup.on && brk->list[i].addr == addr)
      {
	 brk->last = &brk->list[i];
	 brk->last_addr = addr;
	 debug(GDB_CMD, "found mem breakpoint @ 0x%X\n", addr);
	 return 1;
      }
   }

   debug(GDB_CMD, "no mem breakpoint found\n");
   return 0;
}

void gdb_brk_mem_del_all()
{
   size_t                 i;
   vmm_ctrl_dbg_brk_mem_t *brk = &info->vmm.ctrl.dbg.brk.mem;

   for(i=0 ; i<GDB_DFLT_BRK_NR ; i++)
      if(brk->list[i].setup.on)
      {
	 brk->list[i].setup.on = 0;
	 __gdb_brk_mem_restore_insn(&brk->list[i]);
      }
}

void gdb_brk_mem_cleanup()
{
   gdb_brk_mem_del_all();
   gdb_brk_mem_reset_status();
   gdb_release_bp_excp();
}

int gdb_brk_mem_continue()
{
   return gdb_brk_mem_restore_insn();
}

int gdb_brk_mem()
{
   int      mode;
   offset_t vaddr;

   vm_get_code_addr(&vaddr, 0, &mode);

   if(gdb_brk_mem_lookup(vaddr))
      return GDB_PREEMPT;

   return GDB_IGNORE;
}

/*
** Hardware Code/Data breakpoints
**
** we only set global flag
*/
void gdb_brk_hrd_set(offset_t addr, uint64_t type, size_t len)
{
   uint8_t n, mask, conf;
   dr7_reg_t *dr7 = &info->vmm.ctrl.dbg.brk.hrd.dr7;

   conf = (((len&3)<<2)|(type&3)) & 0xf;

   for(n=0 ; n<4 ; n++)
   {
      mask = 1<<(n*2+1);
      if((dr7->blow & mask) == 0)
      {
	 dr7->low |= (conf<<(16+n*4)) | mask;

	 if(type == DR7_COND_X && !gdb_brk_hrd_insn_enabled())
	    gdb_brk_hrd_insn_enable();

	 gdb_brk_hrd_enable();
	 set_dr(n, addr);
	 gdb_ok();
	 debug(GDB_CMD, "installed new hrd breakpoint @ 0x%X\n", get_dr(n));
	 return;
      }
   }

   debug(GDB, "no available hrd breakpoint slots !\n");
   gdb_err_oom();
}

void gdb_brk_hrd_del(offset_t addr, uint64_t type, size_t len)
{
   uint8_t n, mask, conf, _conf, more, more_x;
   dr7_reg_t *dr7 = &info->vmm.ctrl.dbg.brk.hrd.dr7;

   conf = (((len&3)<<2)|(type&3)) & 0xf;
   more = more_x = 0;

   for(n=0 ; n<4 ; n++)
   {
      mask = 1<<(n*2+1);
      if(dr7->blow & mask)
      {
	 _conf = (dr7->low>>(16+n*4)) & 0xf;

	 if(_conf == conf && addr == get_dr(n))
	 {
	    dr7->blow &= ~mask;
	    dr7->low  &= ~(0xf<<(16+n*4));
	    debug(GDB_CMD, "removed hrd breakpoint @ 0x%x\n", addr);
	 }
	 else if(!more_x || !more)
	 {
	    if((_conf&3) == DR7_COND_X)
	       more_x = 1;

	    more = 1;
	 }
      }
   }

   if(!more_x)
   {
      gdb_brk_hrd_insn_disable();

      if(!more)
	 gdb_brk_hrd_disable();
   }

   /* silently drop not found breakpoint */
   gdb_ok();
}

void gdb_brk_hrd_insn_set_rflags(rflags_reg_t *rflags)
{
   info->vmm.ctrl.dbg.brk.hrd.status.rf = rflags->rf;
   rflags->rf = 0;
}

void gdb_brk_hrd_insn_restore_rflags(rflags_reg_t *rflags)
{
   rflags->rf = info->vmm.ctrl.dbg.brk.hrd.status.rf;
}

int gdb_brk_hrd_insn_correct_rflags(rflags_reg_t *rflags)
{
   return (rflags->rf)?0:1;
}

void gdb_brk_hrd_insn_enable()
{
   if(info->vmm.ctrl.dbg.brk.hrd.status.insn)
      return;

   info->vmm.ctrl.dbg.brk.hrd.status.insn = 1;
   gdb_brk_hrd_insn_set_rflags(&__rflags);
   debug(GDB_CMD, "enabled insn hrd breakpoints\n");
}

void gdb_brk_hrd_insn_disable()
{
   if(!info->vmm.ctrl.dbg.brk.hrd.status.insn)
      return;

   info->vmm.ctrl.dbg.brk.hrd.status.insn = 0;
   gdb_brk_hrd_insn_restore_rflags(&__rflags);
   debug(GDB_CMD, "disabled insn hrd breakpoints\n");
}

void gdb_brk_hrd_enable()
{
   if(info->vmm.ctrl.dbg.brk.hrd.status.on)
      return;

   info->vmm.ctrl.dbg.brk.hrd.status.on = 1;
   gdb_protect_db_excp();
   debug(GDB_CMD, "enabled hrd breakpoints\n");
}

void gdb_brk_hrd_disable()
{
   if(!info->vmm.ctrl.dbg.brk.hrd.status.on)
      return;

   info->vmm.ctrl.dbg.brk.hrd.status.on = 0;
   gdb_release_db_excp();
   debug(GDB_CMD, "disabled hrd breakpoints\n");
}

/* static void __gdb_brk_hrd_restore_dr() */
/* { */
/*    offset_t x = 0; */

/*    gdb_clean_dr6(); */
/*    gdb_clean_dr7(); */

/*    set_dr0(x); */
/*    set_dr1(x); */
/*    set_dr2(x); */
/*    set_dr3(x); */
/* } */

static void __gdb_brk_hrd_restore_dr()
{
   gdb_dr6_set_dirty(0);

   set_dr0(info->vm.dr_shadow[0].raw);
   set_dr1(info->vm.dr_shadow[1].raw);
   set_dr2(info->vm.dr_shadow[2].raw);
   set_dr3(info->vm.dr_shadow[3].raw);

   __dr6.low = info->vm.dr_shadow[4].low;
   __dr7.low = info->vm.dr_shadow[5].low;
}

void gdb_brk_hrd_cleanup()
{
   gdb_brk_hrd_reset_status();
   __gdb_brk_hrd_restore_dr();
}

int gdb_brk_hrd_continue()
{
   if(gdb_brk_hrd_last_type() == DR7_COND_X)
   {
      dr7_reg_t *dr7 = &info->vmm.ctrl.dbg.brk.hrd.dr7;
      uint8_t   n = gdb_brk_hrd_last_idx();

      if(dr7->blow & (1<<(n*2+1)))
      {
	 debug(GDB_CMD, "will resume hrd insn breakpoint\n");
	 __rflags.rf = 1;
      }
   }

   return 1;
}

int gdb_brk_hrd_condition()
{
   uint8_t n, conf;
   dr7_reg_t *dr7 = &info->vmm.ctrl.dbg.brk.hrd.dr7;

   for(n=0 ; n<4 ; n++)
      if((__dr6.blow & (1<<n)) && (dr7->blow & (1<<(n*2+1))))
      {
	 conf = (dr7->low>>(16+n*4)) & 0x3;
	 gdb_brk_hrd_set_last(n, conf);
	 gdb_dr6_set_dirty(1);
	 debug(GDB_CMD, "found hrd breakpoint @ 0x%X\n", get_dr(n));
	 return GDB_PREEMPT;
      }

   debug(GDB_CMD, "no hrd breakpoint found\n");
   return GDB_IGNORE;
}

int gdb_brk_hrd()
{
   if(__dr6.bs)
      return gdb_singlestep();

   return gdb_brk_hrd_condition();
}

/*
** Common
*/
int gdb_brk_continue()
{
   if(gdb_last_stop_reason() != GDB_EXIT_TRAP || !__vmexit_on_excp())
      return 1;

   if(__exception_vector == DB_EXCP)
      return gdb_brk_hrd_continue();

   if(__exception_vector == BP_EXCP)
      return gdb_brk_mem_continue();

   return 1;
}

void gdb_brk_enable()
{
   __deny_dr_access();
   gdb_clean_dr6();
   gdb_clean_dr7();
}

void gdb_brk_disable()
{
   gdb_brk_mem_cleanup();
   gdb_brk_hrd_cleanup();
   __allow_dr_access();
}
