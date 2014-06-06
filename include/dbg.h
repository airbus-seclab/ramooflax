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
#ifndef __DBG_H__
#define __DBG_H__

#include <types.h>
#include <dbg_soft.h>
#include <dbg_hard.h>
#include <dbg_evt.h>

/*
** Debug event requestor
*/
#define DBG_REQ_USR      0
#define DBG_REQ_VMM      1

/*
** Some protections
*/
#define ___access_rflags(_access_)		\
   ({						\
      __#_access_##_pushf();			\
      __#_access_##_popf();			\
      __#_access_##_iret();			\
      __#_access_##_icebp();			\
      __#_access_##_soft_int();			\
      __#_access_##_hrdw_int();			\
      __#_access_##_excp();			\
      info->vmm.ctrl.dbg.excp |= (1<<GP_EXCP);	\
   })

#define __protect_rflags()		___access_rflags(deny)
#define __release_rflags()		___access_rflags(allow)

typedef struct ctrl_debugger
{
   uint32_t   excp;
   dbg_soft_t soft;
   dbg_hard_t hard;
   dbg_evt_t  evt;

} __attribute__((packed)) ctrl_dbg_t;

/*
** Functions
*/
void  dbg_resume(uint8_t);
void  dbg_enable();
void  dbg_disable();

#endif
