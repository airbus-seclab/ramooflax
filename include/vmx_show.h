/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#ifndef __VMX_SHOW_H__
#define __VMX_SHOW_H__

/*
** Functions
*/
void vmx_show_basic_info();
void vmx_show_misc_data();
void vmx_show_fixed_pin_ctls();
void vmx_show_fixed_proc_ctls();
void vmx_show_fixed_proc2_ctls();
void vmx_show_fixed_entry_ctls();
void vmx_show_fixed_exit_ctls();
void vmx_show_pin_ctls();
void vmx_show_proc_ctls();
void vmx_show_proc2_ctls();
void vmx_show_entry_ctls();
void vmx_show_exit_ctls();
void vmx_show_ept_cap();

#endif
