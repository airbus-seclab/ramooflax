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
#ifndef __CTRL_EVT_H__
#define __CTRL_EVT_H__

#include <types.h>

typedef int (*ctrl_evt_hdl_t)(arg_t);

#define CTRL_EVT_TYPE_CR_RD    0
#define CTRL_EVT_TYPE_CR_WR    1
#define CTRL_EVT_TYPE_EXCP     2
#define CTRL_EVT_TYPE_BRK      3
#define CTRL_EVT_TYPE_SSTEP    4
#define CTRL_EVT_TYPE_NPF      5
#define CTRL_EVT_TYPE_HYP      6
#define CTRL_EVT_TYPE_CPUID    7

typedef struct controller_event
{
   uint8_t         type;
   arg_t           arg;
   ctrl_evt_hdl_t  hdl;

} __attribute__((packed)) ctrl_evt_t;

int  __ctrl_evt_excp_dbg(uint32_t);

/*
** Functions
*/
int  ctrl_evt_excp(uint32_t);
int  ctrl_evt_cr_rd(uint8_t);
int  ctrl_evt_cr_wr(uint8_t);
int  ctrl_evt_npf();
int  ctrl_evt_hypercall();
int  ctrl_evt_cpuid(uint32_t);

int  ctrl_evt_setup(uint8_t, ctrl_evt_hdl_t, arg_t);
int  ctrl_event();

#endif
