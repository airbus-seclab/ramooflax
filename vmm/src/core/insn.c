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
#include <insn.h>
#include <vmm.h>
#include <emulate.h>
#include <emulate_int.h>
#include <intr.h>
#include <info_data.h>
#include <debug.h>

extern info_data_t *info;

int resolve_hypercall()
{
   return VM_FAIL;
}

int resolve_invd()
{
   asm volatile ("invd");
   return emulate_done(VM_DONE, INVD_INSN_SZ);
}

int resolve_wbinvd()
{
   asm volatile ("wbinvd");
   return emulate_done(VM_DONE, WBINVD_INSN_SZ);
}

int resolve_hlt()
{
   return VM_FAIL;
}

int resolve_icebp()
{
   int rc = emulate_int1();
   return emulate_done(rc, 1);
}

int resolve_default()
{
   return VM_FAIL;
}
