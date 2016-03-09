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

/*
** VM-EXIT handling return codes
*/
#define VM_FAIL              (1<<0)
#define VM_FAULT             (1<<1)
#define VM_DONE              (1<<2)
#define VM_DONE_LET_RIP      (1<<3)
#define VM_NATIVE            (1<<4)
#define VM_IGNORE            (1<<5)
#define VM_INTERN            (1<<6)
#define VM_PARTIAL           (1<<7)

/*
** VM architecture dependant stuff
*/
#ifdef CONFIG_ARCH_AMD
#include <svm_vmm.h>
#include <svm_vm.h>
typedef svm_vmc_t       vmc_t;
typedef svm_bazaar_t    vm_bazaar_t;
#define vm_vmc_init()   svm_vm_init()
#else
#include <vmx_vmm.h>
#include <vmx_vm.h>
typedef vmx_vmc_t       vmc_t;
typedef vmx_bazaar_t    vm_bazaar_t;
#define vm_vmc_init()   vmx_vm_init()
#endif

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
#include <dev_ata.h>
#include <disasm.h>
#include <pci.h>

/*
** General VM settings
*/
#define VM_ENTRY_POINT         RM_BASE_IP

#define vm_set_entry()         *(uint16_t*)VM_ENTRY_POINT = 0x19cd;
/* #define vm_set_entry()         *(uint32_t*)VM_ENTRY_POINT = 0x19cd16cd; */
/* #define vm_set_entry()         *(uint16_t*)VM_ENTRY_POINT = 0xfeeb; */

/*
** VM data structures
*/
typedef struct vm_paging
{
   npg_pml4e_t  *pml4; /* strictly aligned */

} __attribute__((packed)) vm_pgmem_t;

typedef union vm_cpu_skill
{
   struct
   {
      uint32_t  pg_2M:1;         /* nested mappings only     */
      uint32_t  pg_1G:1;         /* nested mappings only     */
      uint32_t  flush_tlb:3;     /* flush vm tlbs non-global */
      uint32_t  flush_tlb_glb:3; /* flush vm all tlbs        */
      uint32_t  paddr_sz:8;      /* max physical addr width */
      uint32_t  vaddr_sz:8;      /* max linear addr width */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) vm_cpu_skill_t;

typedef enum emulate_status
{
   EMU_STS_AVL,
   EMU_STS_NATIVE,
   EMU_STS_DONE
} emu_sts_t;

typedef struct fault_context
{
   /* Last Exception context */
   struct
   {
      uint32_t   err;

   } __attribute__((packed)) excp;

   /* Last Nested Page Fault context */
   struct
   {
      npg_err_t  err;
      offset_t   vaddr;
      offset_t   paddr;

   } __attribute__((packed)) npf;

} __attribute__((packed)) fault_ctx_t;

typedef struct vm_cpu
{
   vm_pgmem_t     pg[NPG_NR]; /* virtual paging tables */
   uint32_t       dflt_excp;  /* default exception mask */
   uint8_t        active_pg;  /* which virtual paging to apply */
   vm_cpu_skill_t skillz;     /* vm cpu skillz */
   uint64_t       max_paddr;  /* maximum physical addr supported */
   uint64_t       max_vaddr;  /* maximum linear addr supported */
   vmc_t          *vmc;       /* hardware virtualization data, strictly aligned */
   gpr64_ctx_t    *gpr;       /* vm GPRs (in vmm stack) */
   fault_ctx_t    fault;      /* last fault context info */
   ud_t           disasm;
   emu_sts_t      emu_sts;
   uint8_t        insn_cache[X86_MAX_INSN_LEN];

} __attribute__((packed)) vm_cpu_t;

typedef struct vm_memory
{
   smap_t      smap;      /* patched bios system memory map */
   uint8_t     a20;       /* a20 state */

} __attribute__((packed)) vm_mem_t;

typedef struct vm_device
{
   vm_mem_t       mem;         /* vm memory definitions */
   ps2_t          ps2;         /* vm virtual ps2 sys ctrl */
   kbd_t          kbd;         /* vm virtual kbd ctrl */
   uart_t         uart;        /* vm com1 proxy */
   uint8_t        pic1_icw2;   /* irq rebase */
   pci_cfg_addr_t pci_addr;    /* last filtered pci addr */
   io_flt_hdl_t   pci_filter;  /* pci data filter handler */
   ata_t          ata[1];      /* vm filtered ata ctrl */

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
#ifdef __INIT__
void  vm_init();
#else
void  vm_get_code_addr(offset_t*, offset_t, int*);
void  vm_get_stack_addr(offset_t*, offset_t, int*);

void  vm_update_rip(offset_t);
void  vm_rewind_rip(offset_t);

int   __vm_remote_access_pmem(vm_access_t*);

int   __vm_recv_mem(cr3_reg_t*, offset_t, size_t);
int   __vm_send_mem(cr3_reg_t*, offset_t, size_t);

int   __vm_read_mem(cr3_reg_t*, offset_t, uint8_t*, size_t);
int   __vm_write_mem(cr3_reg_t*, offset_t, uint8_t*, size_t);

int   vm_read_mem(offset_t, uint8_t*, size_t);
int   vm_write_mem(offset_t, uint8_t*, size_t);

int   vm_enter_rmode();
int   vm_enter_pmode();

void  vm_setup_npg(int);
int   vm_pg_walk(offset_t, offset_t*, size_t*);

struct npg_walk_info;
int   vm_full_walk(offset_t, struct npg_walk_info*);

#endif

#endif
