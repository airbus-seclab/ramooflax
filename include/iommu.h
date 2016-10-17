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
#ifndef __IOMMU_H__
#define __IOMMU_H__

#include <config.h>
#include <types.h>

/*
** Intel VT-d DMAR translation structures
*/
typedef union dmar_base_root_entry
{
   struct
   {
      uint64_t   p:1;
      uint64_t   rsv:11;
      uint64_t   ctp:52;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) dmar_base_roote_t;

typedef struct dmar_root_entry
{
   dmar_base_roote_t;
   uint64_t reserved;

} __attribute__((packed)) dmar_roote_t;

typedef struct dmar_extended_root_entry
{
   dmar_base_roote_t lower;
   dmar_base_roote_t upper;

} __attribute__((packed)) dmar_xroote_t;

#define DMAR_DRHD_CTX_AW_30   0
#define DMAR_DRHD_CTX_AW_39   1
#define DMAR_DRHD_CTX_AW_48   2
#define DMAR_DRHD_CTX_AW_57   3
#define DMAR_DRHD_CTX_AW_64   4

#define DMAR_DRHD_CTX_TT_UR   0
#define DMAR_DRHD_CTX_TT_DT   1
#define DMAR_DRHD_CTX_TT_PT   2


typedef struct dmar_context_entry
{
   union
   {
      struct
      {
         uint64_t   p:1;
         uint64_t   fpd:1;
         uint64_t   t:2;
         uint64_t   rsv0:8;
         uint64_t   slptptr:52;

      } __attribute__((packed));

      raw64_t low;

   } __attribute__((packed));

   union
   {
      struct
      {
         uint64_t   aw:3;
         uint64_t   avail:4;
         uint64_t   rsv1:1;
         uint64_t   did:16;
         uint64_t   rsv2:40;

      } __attribute__((packed));

      raw64_t high;

   } __attribute__((packed));

} __attribute__((packed)) dmar_ctxe_t;

/*
** DMAR Second Level paging structures
*/
#define slt_pg_present(_e_)   (((_e_)->raw & 7) != 0)

typedef union second_level_page_map_level_4_entry
{
   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  ign0:4;
      uint64_t  rsv0:1;
      uint64_t  ign1:3;
      uint64_t  rsv1:1;
      uint64_t  addr:40;   /* bit 12 */
      uint64_t  ign2:10;   /* bit 52 */
      uint64_t  rsv2:1;
      uint64_t  ign3:1;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) sl_pml4e_t;

typedef union second_level_page_directory_pointer_entry
{
   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  ign0:4;
      uint64_t  zero:1;
      uint64_t  ign1:3;
      uint64_t  rsv0:1;
      uint64_t  addr:40;    /* bit 12 */
      uint64_t  ign2:10;    /* bit 52 */
      uint64_t  rsv1:1;     /* bit 62 */
      uint64_t  ign3:1;

   } __attribute__((packed));

   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  emt:3;
      uint64_t  ipat:1;
      uint64_t  ps:1;       /* bit 7 */
      uint64_t  ign0:3;
      uint64_t  snp:1;
      uint64_t  rsv0:18;
      uint64_t  addr:22;    /* bit 30 */
      uint64_t  ign1:10;    /* bit 52 */
      uint64_t  tm:1;       /* bit 62 */
      uint64_t  ign2:1;

   } __attribute__((packed)) page;

   raw64_t;

} __attribute__((packed)) sl_pdpe_t;

typedef union second_level_page_directory_entry_64
{
   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  ign0:4;
      uint64_t  zero:1;
      uint64_t  ign1:3;
      uint64_t  rsv0:1;
      uint64_t  addr:40;    /* bit 12 */
      uint64_t  ign2:10;    /* bit 52 */
      uint64_t  rsv1:1;     /* bit 62 */
      uint64_t  ign3:1;

   } __attribute__((packed));

   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  emt:3;
      uint64_t  ipat:1;
      uint64_t  ps:1;       /* bit 7 */
      uint64_t  ign0:3;
      uint64_t  snp:1;
      uint64_t  rsv0:9;
      uint64_t  addr:31;    /* bit 21 */
      uint64_t  ign1:10;    /* bit 52 */
      uint64_t  tm:1;       /* bit 62 */
      uint64_t  ign2:1;

   } __attribute__((packed)) page;

   raw64_t;

} __attribute__((packed)) sl_pde64_t;

typedef union second_level_page_table_entry_64
{
   struct
   {
      uint64_t  r:1;
      uint64_t  w:1;
      uint64_t  x:1;
      uint64_t  emt:3;
      uint64_t  ipat:1;
      uint64_t  ign0:4;
      uint64_t  snp:1;
      uint64_t  addr:40;    /* bit 12 */
      uint64_t  ign1:10;    /* bit 52 */
      uint64_t  tm:1;       /* bit 62 */
      uint64_t  ign2:1;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) sl_pte64_t;

/*
** Intel VT-d DMAR structure types
*/
#define DMAR_STRUCT_DRHD  0    /* DMA Remapping Hardware Unit Definition */
#define DMAR_STRUCT_RMRR  1    /* Reserved Memory Region Reporting       */
#define DMAR_STRUCT_ATSR  2    /* Root Port ATS Capability Reporting     */
#define DMAR_STRUCT_RHSA  3    /* Remapping Hardware Static Affinity     */
#define DMAR_STRUCT_ANDD  4    /* ACPI Name Space Device Declaration     */

/*
** Generic DMAR structure
*/
typedef struct dmar_structure_generic
{
   uint16_t  type;
   uint16_t  length;

} __attribute__((packed)) dmar_gen_t;

/*
** DMA Remapping Hardware Unit Definition
*/
typedef struct dmar_structure_drhd
{
   dmar_gen_t;

   struct
   {
      uint8_t   include_pci_all:1;
      uint8_t   rsv:7;

   } __attribute__((packed)) flags;

   uint8_t   rsv;
   uint16_t  pci_segment;
   uint64_t  bar;
   uint8_t   device_scope[0];

} __attribute__((packed)) dmar_drhd_t;


/*
** DMA Remapping Hardware Unit Register Set
*/
#define DMAR_DRHD_DOMAIN_SUPPORT_16    0
#define DMAR_DRHD_DOMAIN_SUPPORT_64    1
#define DMAR_DRHD_DOMAIN_SUPPORT_256   2
#define DMAR_DRHD_DOMAIN_SUPPORT_1K    3
#define DMAR_DRHD_DOMAIN_SUPPORT_4K    4
#define DMAR_DRHD_DOMAIN_SUPPORT_16K   5
#define DMAR_DRHD_DOMAIN_SUPPORT_64K   6

#define DMAR_DRHD_CAP_AGAW_39   (1<<1)
#define DMAR_DRHD_CAP_AGAW_48   (1<<2)

typedef union dmar_capability_register
{
   struct
   {
      uint64_t   nd:3;
      uint64_t   afl:1;
      uint64_t   rwbf:1;
      uint64_t   plmr:1;
      uint64_t   phmr:1;
      uint64_t   cm:1;
      uint64_t   sagaw:5;
      uint64_t   rsv0:3;
      uint64_t   mgaw:6;
      uint64_t   zlr:1;
      uint64_t   rsv1:1;
      uint64_t   fro:10;
      uint64_t   sllps:4;
      uint64_t   rsv2:1;
      uint64_t   psi:1;
      uint64_t   nfr:8;
      uint64_t   mamv:6;
      uint64_t   dwd:1;
      uint64_t   drd:1;
      uint64_t   fl1gp:1;
      uint64_t   rsv3:2;
      uint64_t   pi:1;
      uint64_t   rsv4:4;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) dmar_cap_reg_t;

typedef union dmar_extended_capabilty_register
{
   struct
   {
      uint64_t   c:1;
      uint64_t   qi:1;
      uint64_t   dt:1;
      uint64_t   ir:1;
      uint64_t   eim:1;
      uint64_t   rsv0:1;
      uint64_t   pt:1;
      uint64_t   sc:1;
      uint64_t   iro:10;
      uint64_t   rsv1:2;
      uint64_t   mhmv:4;
      uint64_t   ecs:1;
      uint64_t   mts:1;
      uint64_t   nest:1;
      uint64_t   dis:1;
      uint64_t   rsv2:1;
      uint64_t   prs:1;
      uint64_t   ers:1;
      uint64_t   srs:1;
      uint64_t   rsv3:1;
      uint64_t   nwfs:1;
      uint64_t   eafs:1;
      uint64_t   pss:5;
      uint64_t   pasid:1;
      uint64_t   dit:1;
      uint64_t   pds:1;
      uint64_t   rsv4:21;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) dmar_xcap_reg_t;

typedef union dmar_global_command_status_register
{
   struct
   {
      uint32_t   rsvd0:23;
      uint32_t   cfi:1;
      uint32_t   irtp:1;
      uint32_t   ire:1;
      uint32_t   qie:1;
      uint32_t   wbf:1;
      uint32_t   eafl:1;
      uint32_t   fl:1;
      uint32_t   rtp:1;
      uint32_t   te:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) dmar_cmd_sts_reg_t;

typedef union dmar_root_table_address_register
{
   struct
   {
      uint64_t  rsv:11;
      uint64_t  rtt:1;
      uint64_t  rta:52;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) dmar_rtar_reg_t;

typedef union dmar_context_command_register
{
   struct
   {
      uint64_t   did:16;
      uint64_t   sid:16;
      uint64_t   fm:2;
      uint64_t   rsv:25;
      uint64_t   caig:2;
      uint64_t   cirg:2;
      uint64_t   icc:1;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) dmar_ctx_cmd_reg_t;

typedef union dmar_fault_status_register
{
   struct
   {
      uint32_t   pfo:1;
      uint32_t   ppf:1;
      uint32_t   afo:1;
      uint32_t   apf:1;
      uint32_t   iqe:1;
      uint32_t   ice:1;
      uint32_t   ite:1;
      uint32_t   pro:1;
      uint32_t   fri:8;
      uint32_t   rsv:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) dmar_flt_sts_reg_t;

typedef union dmar_fault_event_control_register
{
   struct
   {
      uint32_t   rsv:30;
      uint32_t   ip:1;
      uint32_t   im:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) dmar_flt_evc_reg_t;

typedef union dmar_fault_event_data_register
{
   struct
   {
      uint32_t   vector:8;
      uint32_t   dm:1;
      uint32_t   rsv:23;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) dmar_flt_evd_reg_t;

typedef union dmar_fault_event_address_register
{
   struct
   {
      uint32_t   ign:2;
      uint32_t   dm:1;
      uint32_t   rh:1;
      uint32_t   rsv:8;
      uint32_t   did:8;
      uint32_t   fee:12;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) dmar_flt_eva_reg_t;

typedef union dmar_fault_event_upper_register
{
   struct
   {
      uint32_t   rsv:8;
      uint32_t   did:24;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) dmar_flt_evu_reg_t;

typedef struct dmar_fault_recording_register
{
   union
   {
      struct
      {
         uint64_t  rsv0:12;
         uint64_t  fi:52;

      } __attribute__((packed));

      raw64_t low;

   } __attribute__((packed));

   union
   {
      struct
      {
         uint64_t   sid:16;
         uint64_t   rsv1:13;
         uint64_t   priv:1;
         uint64_t   exe:1;
         uint64_t   pp:1;
         uint64_t   fr:8;
         uint64_t   pv:20;
         uint64_t   at:2;
         uint64_t   t:1;
         uint64_t   f:1;

      } __attribute__((packed));

      raw64_t high;

   } __attribute__((packed));

} __attribute__((packed)) dmar_flt_rec_reg_t;


/* typedef union dmar_xxx_register */
/* { */
/*    struct */
/*    { */

/*    } __attribute__((packed)); */

/*    rawXX_t; */

/* } __attribute__((packed)) dmar_xx_reg_t; */

typedef struct dmar_drhd_registers
{
   uint32_t            version;
   uint32_t            rsv0;

   dmar_cap_reg_t      cap;
   dmar_xcap_reg_t     xcap;
   dmar_cmd_sts_reg_t  cmd;
   dmar_cmd_sts_reg_t  sts;
   dmar_rtar_reg_t     root_addr;
   dmar_ctx_cmd_reg_t  ctx_cmd;
   uint32_t            rsv1;

   dmar_flt_sts_reg_t  fault_sts;
   dmar_flt_evc_reg_t  fault_evc;
   dmar_flt_evd_reg_t  fault_evd;
   dmar_flt_eva_reg_t  fault_eva;
   dmar_flt_evu_reg_t  fault_evu;

   uint64_t            rsv2;
   uint64_t            rsv3;
   uint64_t            adv_fault_log;
   uint32_t            rsv4;

   uint32_t            pmem_en;
   uint32_t            low_mem_base;
   uint32_t            low_mem_limit;
   uint64_t            high_mem_base;
   uint64_t            high_mem_limit;

   uint64_t            inv_queue_head;
   uint64_t            inv_queue_tail;
   uint64_t            inv_queue_addr;
   uint32_t            rsv5;

   /* Many more */

} __attribute__((packed)) dmar_drhd_reg_t;

/*
** I/O MMU information data
*/
typedef struct dmar_drhd_information
{
   volatile dmar_drhd_reg_t *reg; /* Memory Mapped registers */

   /* cached value of these registers */
   dmar_cap_reg_t   cap;
   dmar_xcap_reg_t xcap;
   uint8_t         agaw;

   /* we only have one root/context/slt for every devices */
   dmar_roote_t    *root;
   dmar_ctxe_t     *ctx;
   sl_pml4e_t      *slt;

} __attribute__((packed)) dmar_drhd_info_t;

typedef struct iommu_information_data
{
   dmar_drhd_info_t all;  /* DRHD for PCI-ALL */

} __attribute__((packed)) iommu_info_t;


/*
** Functions
*/
#ifdef __INIT__
void iommu_init();
#endif

void dmar_drhd_fault_check(dmar_drhd_info_t*);


#endif
