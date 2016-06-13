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
#ifndef __PCI_H__
#define __PCI_H__

#include <types.h>
#include <asm.h>
#include <io.h>

/*
** pci device configuration space
*/
#define PCI_CFG_DEV_VDR_OFFSET      0x0    /* dev & vendor id */
#define PCI_CFG_CMD_STS_OFFSET      0x4    /* status & command */
#define PCI_CFG_CLASS_REV_OFFSET    0x8    /* class code & revision */
#define PCI_CFG_BAR_OFFSET          0x10   /* bar set */
#define PCI_CFG_CAP_OFFSET          0x34   /* capabilities pointer */
#define PCI_CFG_INT_OFFSET          0x3c   /* interrupts info */

#define PCI_CFG_VENDOR_INTEL        0x8086
#define PCI_CFG_VENDOR_NVIDIA       0x10de
#define PCI_CFG_DEVICE_INVALID      0xffff

typedef union pci_config_space_dev_vendor
{
   struct
   {
      uint16_t vendor;
      uint16_t device;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_dev_vdr_t;

/*
** pci config space command and status register
*/
#define PCI_CFG_STATUS_DEVSEL_FAST  0
#define PCI_CFG_STATUS_DEVSEL_MEDM  1
#define PCI_CFG_STATUS_DEVSEL_SLOW  2

typedef union pci_config_space_command
{
   struct
   {
      uint16_t  io:1;
      uint16_t  mm:1;
      uint16_t  bus_master:1;
      uint16_t  sp_cycles:1;
      uint16_t  mm_wie:1;
      uint16_t  vga_snoop:1;
      uint16_t  parity:1;
      uint16_t  stepping:1;
      uint16_t  serr:1;
      uint16_t  b2b:1;
      uint16_t  rsvd:6;

   } __attribute__((packed));

   uint16_t raw;

} __attribute__((packed)) pci_cfg_cmd_t;

typedef union pci_config_space_status
{
   struct
   {
      uint16_t  rsv0:4;
      uint16_t  cap_list:1;
      uint16_t  frq_66:1;
      uint16_t  rsv1:1;
      uint16_t  fast_b2b:1;
      uint16_t  mst_data_p:1;
      uint16_t  devsel:2;
      uint16_t  sig_tgt_abort:1;
      uint16_t  rcv_tgt_abort:1;
      uint16_t  rcv_mst_abort:1;
      uint16_t  sig_sys_err:1;
      uint16_t  dtc_p_err:1;

   } __attribute__((packed));

   uint16_t raw;

} __attribute__((packed)) pci_cfg_sts_t;

typedef union pci_config_space_command_status
{
   struct
   {
      pci_cfg_cmd_t cmd;
      pci_cfg_sts_t sts;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_cmd_sts_t;

typedef union pci_config_space_class_revision
{
   struct
   {
      uint32_t     revision:8;
      uint32_t     class:24;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_class_rev_t;


#define PCI_CFG_MEM_BAR_64   2

typedef union pci_config_space_bar
{
   struct
   {
      uint32_t   io:1;        /* 0: mem, 1: i/o */
      uint32_t   type:2;      /* 0: 32 bits, 2:64 bits */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_bar_t;


typedef union pci_config_space_capability
{
   struct
   {
      uint32_t   id:8;
      uint32_t   next:8;
      uint32_t   data:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_cap_t;

/*
** i/o access
*/
#define PCI_CONFIG_ADDR 0xcf8
#define PCI_CONFIG_DATA 0xcfc

typedef union pci_config_address_register
{
   struct
   {
      uint32_t   reg:8;
      uint32_t   fnc:3;
      uint32_t   dev:5;
      uint32_t   bus:8;
      uint32_t   rsvd:7;
      uint32_t   enbl:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_addr_t;

/*
** Generic pci data structure
*/
typedef struct pci_config_value
{
   pci_cfg_addr_t addr;

   union
   {
      pci_cfg_dev_vdr_t     dv;
      pci_cfg_cmd_sts_t     cs;
      pci_cfg_class_rev_t   cr;
      pci_cfg_bar_t         br;
      pci_cfg_cap_t         cp;

      raw32_t data;

   } __attribute__((packed));

} __attribute__((packed)) pci_cfg_val_t;

#define pci_cfg_read(_pci)                              \
   ({                                                   \
      outl((_pci)->addr.raw, PCI_CONFIG_ADDR);          \
      (_pci)->data.raw = inl(PCI_CONFIG_DATA);          \
   })

#define pci_cfg_write(_pci)                             \
   ({                                                   \
      outl((_pci)->addr.raw, PCI_CONFIG_ADDR);          \
      outl((_pci)->data.raw, PCI_CONFIG_DATA);          \
   })

#define pci_cfg_read64(_pci)                            \
   ({                                                   \
      pci_cfg_val_t xpci;                               \
      raw64_t       data;                               \
                                                        \
      xpci.addr.raw = (_pci)->addr.raw;                 \
                                                        \
      pci_cfg_read(&xpci);                              \
      data.low = xpci.data.raw;                         \
                                                        \
      xpci.addr.reg += 4;                               \
      pci_cfg_read(&xpci);                              \
      data.high = xpci.data.raw;                        \
                                                        \
      data.raw;                                         \
   })

/*
** Functions
*/
typedef int (*pci_search_t)(pci_cfg_val_t*, uint32_t);

int  pci_check_cap(pci_cfg_val_t*, uint32_t);
int  pci_check_dvd(pci_cfg_val_t*, uint32_t);

int  pci_read_bar(pci_cfg_val_t*, uint32_t);
int  pci_search(pci_search_t, uint32_t, size_t, pci_cfg_val_t*);
void pci_dump_registers(pci_cfg_val_t*);

int  dev_pci(io_insn_t*);

#endif
