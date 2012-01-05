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

static int local_evt_cr_rd(arg_t __unused__ arg)
{
   return 1;
}

static int local_evt_cr_wr(arg_t __unused__ arg)
{
   return 1;
}

static int local_evt_excp(arg_t __unused__ arg)
{
   return 1;
}

static int local_evt_brk(arg_t __unused__ arg)
{
   return 1;
}

static int local_evt_stp(arg_t __unused__ arg)
{
   return 1;
}

ctrl_evt_hdl_t ctrl_evt_dft_hdl[] = {
   local_evt_cr_rd,
   local_evt_cr_wr,
   local_evt_excp,
   local_evt_brk,
   local_evt_stp,
};
