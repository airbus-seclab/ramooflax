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
#ifndef __VM_H__
#define __VM_H__

#include <config.h>
#include <types.h>
#include <realmem.h>
#include <paging.h>
#include <smap.h>
#include <excp.h>
#include <insn.h>
#include <dev.h>
#include <dev_ps2.h>
#include <dev_kbd.h>
#include <dev_pic.h>
#include <dev_uart.h>

/*
** General VM settings
*/
#define VM_ENTRY_POINT         RM_BASE_IP

#define vm_set_entry()         *(uint16_t*)VM_ENTRY_POINT = 0x19cd;
//#define vm_set_entry()         *(uint32_t*)VM_ENTRY_POINT = 0x19cd16cd;
//#define vm_set_entry()         *(uint16_t*)VM_ENTRY_POINT = 0xfeeb;

/*
** VM architecture dependant stuff
*/
#ifdef CONFIG_ARCH_AMD
#include <svm_vmm.h>
#include <svm_vm.h>
typedef svm_vmc_t     vmc_t;
typedef svm_bazaar_t  vm_bazaar_t;
#else
#include <vmx_vmm.h>
#include <vmx_vm.h>
typedef vmx_vmc_t     vmc_t;
typedef vmx_bazaar_t  vm_bazaar_t;
#endif

/*
** VM data structures
*/
typedef struct vm_paging
{
   npg_pml4e_t  *pml4; /* strictly aligned */
   npg_pdp_t    *pdp;  /* strictly aligned */

} __attribute__((packed)) vm_pgmem_t;

typedef union vm_cpu_skill
{
   struct
   {
      uint32_t  pg_2M:1;         /* nested mappings only     */
      uint32_t  pg_1G:1;         /* nested mappings only     */
      uint32_t  flush_tlb:3;     /* flush vm tlbs non-global */
      uint32_t  flush_tlb_glb:3; /* flush vm all tlbs        */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vm_cpu_skill_t;

typedef struct vm_cpu
{
   vm_pgmem_t     pg;        /* virtual paging tables */
   uint32_t       dflt_excp; /* default exception mask */
   uint8_t        emu_done;  /* emulation engine called */
   vm_cpu_skill_t skillz;    /* vm cpu skillz */
   vmc_t          *vmc;      /* hardware virtualization data, strictly aligned */
   gpr64_ctx_t    *gpr;      /* vm GPRs (in vmm stack) */
   uint8_t        insn_cache[X86_MAX_INSN_LEN];

} __attribute__((packed)) vm_cpu_t;

typedef struct vm_memory
{
   smap_t      smap;      /* patched bios system memory map */
   uint8_t     a20;       /* a20 state */

} __attribute__((packed)) vm_mem_t;

typedef struct vm_device
{
   vm_mem_t    mem;       /* vm memory definitions */
   ps2_t       ps2;       /* vm virtual ps2 sys ctrl */
   kbd_t       kbd;       /* vm virtual kbd ctrl */
   uart_t      uart;      /* vm com1 proxy */
   uint8_t     pic1_icw2; /* irq rebase */

} __attribute__((packed)) vm_dev_t;

typedef struct vm
{
   vm_cpu_t    cpu;       /* vm cpu info */
   vm_dev_t    dev;       /* vm devices info */

   vm_bazaar_t;           /* vm additional control data */

} __attribute__((packed)) vm_t;

struct vm_access;

typedef int (*vm_access_mem_op_t)(struct vm_access*);

typedef struct vm_access
{
   cr3_reg_t          *cr3;      /* translate virt to phys vm addr */
   offset_t           addr;      /* vm addr (virt or phys) */
   void               *data;
   size_t             len;       /* total bytes */
   uint8_t            wr;        /* write access */
   vm_access_mem_op_t operator;  /* memcpy, remote, io */

} __attribute__((packed)) vm_access_t;

/*
** Functions
*/
#ifndef __INIT__
void  vm_get_code_addr(offset_t*, offset_t, int*);
void  vm_get_stack_addr(offset_t*, offset_t, int*);

void  vm_update_rip(offset_t);
void  vm_rewind_rip(offset_t);

int   __vm_local_access_pmem(vm_access_t*);
int   __vm_remote_access_pmem(vm_access_t*);
int   __vm_access_mem(vm_access_t*);

int   __vm_recv_mem(cr3_reg_t*, offset_t, size_t);
int   __vm_send_mem(cr3_reg_t*, offset_t, size_t);

int   __vm_read_mem(cr3_reg_t*, offset_t, uint8_t*, size_t);
int   __vm_write_mem(cr3_reg_t*, offset_t, uint8_t*, size_t);

int   vm_read_mem(offset_t, uint8_t*, size_t);
int   vm_write_mem(offset_t, uint8_t*, size_t);

int   vm_enter_rmode();
int   vm_enter_pmode();
#endif

#endif
