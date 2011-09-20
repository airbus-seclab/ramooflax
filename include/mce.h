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
#ifndef __MCE_H__
#define __MCE_H__

#include <msr.h>

/*
** IA32_MCG_CAP
*/
typedef union ia32_mcg_cap
{
   struct
   {
      uint64_t    count:8;
      uint64_t    ctl_p:1;
      uint64_t    ext_p:1;
      uint64_t    cmci_p:1;
      uint64_t    tes_p:1;
      uint64_t    rsvd:4;
      uint64_t    ext_cnt:8;
      uint64_t    ser_p:1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) ia32_mcg_cap_t;

#define IA32_MCG_CAP                0x179UL
#define rd_msr_ia32_mcg_cap(_m)     rd_msr64(IA32_MCG_CAP, (_m).edx, (_m).eax)

/*
** IA32_MCi_STATUS
*/
typedef union ia32_mci_status
{
   struct
   {
      uint64_t    mca_err:16;
      uint64_t    spc_err:16;
      uint64_t    other:6;
      uint64_t    corr_err_cnt:15;
      uint64_t    thr_err_sts:2;
      uint64_t    ar:1;
      uint64_t    s:1;
      uint64_t    pcc:1;
      uint64_t    addr_v:1;
      uint64_t    misc_v:1;
      uint64_t    en:1;
      uint64_t    uc:1;
      uint64_t    over:1;
      uint64_t    val:1;

   } __attribute__((packed));

   msr_t;

} __attribute__((packed)) ia32_mci_status_t;

#define IA32_MCi_STATUS                0x400UL
#define rd_msr_ia32_mci_status(_m,_n)  rd_msr64(IA32_MCi_STATUS+(_n), (_m).edx, (_m).eax)

#endif
