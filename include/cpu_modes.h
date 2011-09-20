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
#ifndef __CPU_MODES_H__
#define __CPU_MODES_H__

/* auto-includes in vmx_vm.h and svm_vm.h */

#define _xx_lmode()       (__efer.lma)
#define _xx_pmode()       (__cr0.pe)

/* long mode: 64 bits mode and compatibility mode 32:16 */
#define __lmode64()       (_xx_lmode() &&  __cs.attributes.l)
#define __lmode32()       (_xx_lmode() && !__cs.attributes.l &&  __cs.attributes.d)
#define __lmode16()       (_xx_lmode() && !__cs.attributes.l && !__cs.attributes.d)

/* legacy mode: real mode, protected mode 32:16, virtual8086 mode */
#define __pmode32()       (_xx_pmode() && !__rflags.vm &&  __cs.attributes.d)
#define __pmode16()       (_xx_pmode() && !__rflags.vm && !__cs.attributes.d)

#define __rmode()         (!_xx_lmode() && !_xx_pmode())
#define __v86mode()       (!_xx_lmode() &&  _xx_pmode() && __rflags.vm)
#define __legacy32()      (!_xx_lmode() &&  __pmode32())
#define __legacy16()      (!_xx_lmode() &&  __pmode16())

#define __paging()        (__cr0.pg)

#endif
