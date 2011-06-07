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
#ifndef __CTRL_H__
#define __CTRL_H__

#include <types.h>
#include <gdb.h>
#include <ctrl_hdl.h>

#define CTRL_RATIO               (10UL)

typedef union vmm_controller_user_status
{
   struct
   {
      uint8_t          xx:1;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) vmm_ctrl_usr_sts_t;

typedef struct vmm_controller_user
{
   vmm_ctrl_usr_sts_t  status;
   uint32_t            excp;   /* user requested excp     intercepts */
   uint32_t            cr_rd;  /* user requested read  CR intercepts */
   uint32_t            cr_wr;  /* user requested write CR intercepts */

} __attribute__((packed)) vmm_ctrl_usr_t;

typedef struct vmm_controller
{
   vmm_ctrl_dbg_t  dbg;
   vmm_ctrl_usr_t  usr;
   raw64_t         vmexit_cnt;

} __attribute__((packed)) vmm_ctrl_t;

/*
** Functions
*/
#define ctrl_activity_set(_x_)     (info->vmm.ctrl.dbg.status.vm_hlt=_x_)
#define ctrl_active()              (info->vmm.ctrl.dbg.status.vm_hlt)
#define ctrl_release_vm()          ctrl_activity_set(0)
#define ctrl_set_active()          ctrl_activity_set(1)

void    vmm_ctrl();
void    ctrl_logic();

#endif
