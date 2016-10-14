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

#ifndef __ACPI_H__
#define __ACPI_H__

#include <config.h>
#include <types.h>
#include <mbi.h>
#include <io.h>

/*
** Root System Description Pointer RSDP
*/
#define ACPI_RSDP 0x2052545020445352 /* "RSD PTR " */

typedef struct acpi_root_system_description_pointer
{
   uint64_t  signature;
   uint8_t   checksum;
   uint8_t   oemid[6];
   uint8_t   revision;
   uint32_t  rsdt;
   uint32_t  length;
   uint64_t  xsdt;
   uint8_t   xchk;
   uint8_t   rsv[3];

} __attribute__((packed)) acpi_rsdp_t;

/*
** Generic System Description Table header
*/
typedef struct acpi_system_description_table_header
{
   uint32_t  signature;
   uint32_t  length;
   uint8_t   revision;
   uint8_t   checksum;
   uint8_t   oemid[6];
   uint8_t   oemtblid[8];
   uint32_t  oemrev;
   uint32_t  creatorid;
   uint32_t  creatorrev;

} __attribute__((packed)) acpi_sys_hdr_t;

/*
** eXtended System Description Table XSDT
*/
#define ACPI_XSDT 0x54445358

typedef struct acpi_extended_system_description_table
{
   acpi_sys_hdr_t;
   uint64_t entry[0];

} __attribute__((packed)) acpi_xsdt_t;

/*
** Fixed ACPI Description Table FADT
*/
#define ACPI_FADT 0x50434146

typedef union acpi_fadt_flags
{
   struct
   {
      uint32_t  wbinvd:1;
      uint32_t  wbinvd_flush:1;
      uint32_t  proc_c1:1;
      uint32_t  p_lvl2_up:1;
      uint32_t  pwr_button:1;
      uint32_t  slp_button:1;
      uint32_t  fix_rtc:1;
      uint32_t  rtc_s4:1;
      uint32_t  tmr_val_ext:1;
      uint32_t  dck_cap:1;
      uint32_t  reset_reg_sup:1;
      uint32_t  sealed_case:1;
      uint32_t  headless:1;
      uint32_t  cpu_sw_slp:1;
      uint32_t  pci_exp_wak:1;
      uint32_t  platform_clk:1;
      uint32_t  s4_rtc_sts:1;
      uint32_t  remote_power_on:1;
      uint32_t  force_apic_cluster:1;
      uint32_t  force_apic_phys:1;
      uint32_t  hw_reduced:1;
      uint32_t  low_power_s0:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) acpi_fadt_flg_t;

typedef struct acpi_fixed_description_table
{
   acpi_sys_hdr_t;

   uint32_t         facs;
   uint32_t         dsdt;
   uint8_t          rsv1;
   uint8_t          pm_profile;
   uint16_t         sci;
   uint32_t         smi_cmd;
   uint8_t          acpi_en;
   uint8_t          acpi_dis;
   uint8_t          s4bios;
   uint8_t          pstate_cnt;
   uint32_t         pm1a_evt_blk;
   uint32_t         pm1b_evt_blk;
   uint32_t         pm1a_cnt_blk;
   uint32_t         pm1b_cnt_blk;
   uint32_t         pm2_cnt_blk;
   uint32_t         pm_tmr_blk;
   uint32_t         gpe0_blk;
   uint32_t         gpe1_blk;
   uint8_t          pm1_evt_len;
   uint8_t          pm1_cnt_len;
   uint8_t          pm2_cnt_len;
   uint8_t          pm_tmr_len;
   uint8_t          gpe0_blk_len;
   uint8_t          gpe1_blk_len;
   uint8_t          gpe1_base;
   uint8_t          cst_cnt;
   uint16_t         p_lvl2_lat;
   uint16_t         p_lvl3_lat;
   uint16_t         flush_size;
   uint16_t         flush_stride;
   uint8_t          duty_offset;
   uint8_t          duty_width;
   uint8_t          day_alrm;
   uint8_t          mon_alrm;
   uint8_t          century;
   uint16_t         iapc_boot_arch;
   uint8_t          rsv2;
   acpi_fadt_flg_t  flags;
   uint8_t          reset_reg[12];
   uint8_t          reset_val;
   uint16_t         arm_boot_arch;
   uint8_t          fadt_minor;
   uint64_t         x_facs;
   uint64_t         x_dsdt;
   uint8_t          x_pm1a_evt_blk[12];
   uint8_t          x_pm1b_evt_blk[12];
   uint8_t          x_pm1a_cnt_blk[12];
   uint8_t          x_pm1b_cnt_blk[12];
   uint8_t          x_pm2_cnt_blk[12];
   uint8_t          x_pm_tmr_blk[12];
   uint8_t          x_gpe0_blk[12];
   uint8_t          x_gpe1_blk[12];
   uint8_t          sleep_ctl_reg[12];
   uint8_t          sleep_sts_reg[12];
   uint64_t         hypervisor_id;

} __attribute__((packed)) acpi_fadt_t;

/*
** Device Memory Access Remapping Table DMAR
*/
#define ACPI_DMAR 0x52414d44

typedef struct acpi_dma_remapping_table
{
   acpi_sys_hdr_t;

   uint8_t   host_addr_width;
   uint8_t   flags;
   uint8_t   rsv[10];
   uint8_t   structures[0];

} __attribute__((packed)) acpi_dmar_t;

/*
** Firmware ACPI Control Structure FACS
*/
#define ACPI_FACS 0x53434146

typedef union acpi_facs_flags
{
   struct
   {
      uint32_t  s4bios:1;
      uint32_t  wake64:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) acpi_facs_flg_t;

typedef union acpi_facs_ospm_flags
{
   struct
   {
      uint32_t  wake64:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) acpi_facs_ospm_flg_t;

typedef struct acpi_firmware_control_structure
{
   uint32_t              signature;
   uint32_t              length;
   uint32_t              hardware_signature;
   uint32_t              waking_vector;
   uint32_t              global_lock;
   acpi_facs_flg_t       flags;
   uint64_t              x_waking_vector;
   uint8_t               version;
   uint8_t               reserved[3];
   acpi_facs_ospm_flg_t  ospm_flags;

} __attribute__((packed)) acpi_facs_t;

/*
** PM1 Control Registers
*/
typedef union acpi_pm1_control_register
{
   struct
   {
      uint16_t  sci_en:1;
      uint16_t  bm_rld:1;
      uint16_t  gbl_rls:1;
      uint16_t  rsv:6;
      uint16_t  ign:1;
      uint16_t  slp_tp:3;
      uint16_t  slp_en:1;

   } __attribute__((packed));

   raw16_t;

} __attribute__((packed)) acpi_pm1_ctl_t;


/*
** VMM ACPI info object
*/
typedef struct acpi_information
{
   acpi_rsdp_t *rsdp;
   acpi_xsdt_t *xsdt;
   acpi_fadt_t *fadt;
   acpi_facs_t *facs;
   acpi_dmar_t *dmar;
   uint32_t     pm1_ctl_port;
   uint8_t      s3;

} __attribute__((packed)) acpi_info_t;


/*
** Functions
*/
#ifdef __INIT__
void acpi_init(mbi_t*);
#endif

int  acpi_wake_up();
int  dev_acpi_pm1_ctl(io_insn_t*);

#endif
