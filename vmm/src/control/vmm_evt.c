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
#include <ctrl_evt.h>
#include <dbg.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static int vmm_evt_excp(arg_t __unused__ arg)
{
   return VM_IGNORE;
}

static int vmm_evt_hyp(arg_t __unused__ arg)
{
   return VM_IGNORE;
}

static int vmm_evt_npf(arg_t __unused__ arg)
{
   return VM_IGNORE;
}

static int vmm_evt_wcr(arg_t __unused__ arg)
{
   return VM_IGNORE;
}

static int vmm_evt_suspend(arg_t __unused__ arg)
{
   return VM_IGNORE;
}

ctrl_evt_hdl_t ctrl_evt_vmm_hdl[] = {
   vmm_evt_excp,
   vmm_evt_hyp,
   vmm_evt_npf,
   vmm_evt_wcr,
   vmm_evt_suspend,
};
