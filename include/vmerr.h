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
#ifndef __VM_ERROR_H__
#define __VM_ERROR_H__

/*
** VM-EXIT handling return codes
** Do not use 0 value, binary tests
** wouldn't work (ie. rc & VM_FAIL)
*/
#define VM_FAIL              (1<<0)
#define VM_FAULT             (1<<1)
#define VM_DONE              (1<<2)
#define VM_DONE_LET_RIP      (1<<3)
#define VM_NATIVE            (1<<4)
#define VM_IGNORE            (1<<5)
#define VM_INTERN            (1<<6)
#define VM_PARTIAL           (1<<7)

/* used for device status sharing vm error codes */
#define VM_DEV_BASE          (1<<16)

#endif
