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
#include <dbg_hard.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void dbg_hard_protect_dr()
{
   debug(DBG_HARD_BRK, "hard protect dr\n");
   /*
   ** XXX: protect DEBUG_CTL_MSR
   ** vmx: load/save dbgctl + msr intercept
   ** svm: virt lbr stores dbgctl into vmcb
   **      else deny rw msr IA32_DEBUG_CTL_MSR
   */
   __pre_access(__dr6);
   __pre_access(__dr7);
   __deny_dr_access();

   info->vm.dr_shadow[0].raw = get_dr0();
   info->vm.dr_shadow[1].raw = get_dr1();
   info->vm.dr_shadow[2].raw = get_dr2();
   info->vm.dr_shadow[3].raw = get_dr3();
   info->vm.dr_shadow[4].low = __dr6.low;
   info->vm.dr_shadow[5].low = __dr7.low;

   dbg_hard_dr6_clean();
   dbg_hard_brk_dr7_clean();
}

static void dbg_hard_release_dr()
{
   debug(DBG_HARD_BRK, "hard release dr\n");

   /* XXX: release DBG_CTL_MSR */

   /* XXX: vmm area check ? */
   set_dr0(info->vm.dr_shadow[0].raw);
   set_dr1(info->vm.dr_shadow[1].raw);
   set_dr2(info->vm.dr_shadow[2].raw);
   set_dr3(info->vm.dr_shadow[3].raw);

   __dr6.low = info->vm.dr_shadow[4].low;
   __dr7.low = info->vm.dr_shadow[5].low;

   dbg_hard_set_dr6_dirty(0);

   __post_access(__dr6);
   __post_access(__dr7);
   __allow_dr_access();
}

void dbg_hard_protect()
{
   if(!dbg_hard_brk_enabled() && !dbg_hard_stp_enabled())
      return;

   if(info->vmm.ctrl.dbg.excp & (1<<DB_EXCP))
      return;

   info->vmm.ctrl.dbg.excp |= (1<<DB_EXCP);
   ctrl_traps_set_update(1);

   debug(DBG_HARD, "installed #DB intercept (%d|%d)\n"
         ,dbg_hard_stp_enabled()
         ,dbg_hard_brk_enabled());
}

void dbg_hard_release()
{
   if(dbg_hard_brk_enabled() || dbg_hard_stp_enabled())
      return;

   if(!(info->vmm.ctrl.dbg.excp & (1<<DB_EXCP)))
      return;

   info->vmm.ctrl.dbg.excp &= ~(1<<DB_EXCP);
   ctrl_traps_set_update(1);

   debug(DBG_HARD, "removed #DB intercept (%d|%d)\n"
         ,dbg_hard_stp_enabled()
         ,dbg_hard_brk_enabled());
}

void dbg_hard_setup()
{
   dbg_hard_protect_dr();
}

void dbg_hard_reset()
{
   dbg_hard_brk_reset();
   dbg_hard_stp_reset();
   dbg_hard_release_dr();
}
