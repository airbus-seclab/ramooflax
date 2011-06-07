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
#ifndef __PCI_H__
#define __PCI_H__

#include <types.h>
#include <asm.h>

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

#define pci_cfg_read(addr)			\
   ({						\
      outl(addr.raw, PCI_CONFIG_ADDR);		\
      inl(PCI_CONFIG_DATA);			\
   })

#define pci_cfg_write(addr,data)		\
   ({						\
      outl(addr.raw, PCI_CONFIG_ADDR);		\
      outl(data, PCI_CONFIG_DATA);		\
   })

/*
** Usefull pci data structure
*/
typedef struct pci_config_value
{
   pci_cfg_addr_t addr;
   raw32_t        data;

} __attribute__((packed)) pci_cfg_val_t;

/*
** pci device configuration space
*/
#define PCI_CFG_DEV_VDR_OFFSET      0x0    /* dev & vendor id */
#define PCI_CFG_CMD_STS_OFFSET      0x4    /* status & command */
#define PCI_CFG_CLASS_REV_OFFSET    0x8    /* class code & revision */
#define PCI_CFG_BAR_OFFSET          0x10   /* bar set */
#define PCI_CFG_CAP_OFFSET          0x34   /* capabilities pointer */

#define PCI_CFG_VENDOR_NVIDIA       0x10de

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

typedef struct pci_config_space_class_code
{
   uint8_t  pi;
   uint8_t  sub;
   uint8_t  base;

} __attribute__((packed)) pci_cfg_class_t;

typedef union pci_config_space_class_revision
{
   struct
   {
      uint8_t          revision;
      pci_cfg_class_t  class;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_class_rev_t;

typedef union pci_config_space_bar_mem
{
   struct
   {
      uint32_t   io:1;        /* 0: mem based, 1: I/O based */
      uint32_t   type:2;      /* 0: 32 bits, 2: 64 bits */
      uint32_t   rsv:5;       /* reserved in EHCI spec */
      uint32_t   addr:24;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_bar_mem_t;

typedef union pci_config_space_bar_io
{
   struct
   {
      uint32_t   one:1;
      uint32_t   zero:1;
      uint32_t   base_addr:30;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_bar_io_t;

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
** Specific ehci pci config space
*/
#define PCI_CFG_EHCI_INT_LINE_OFFSET 0x3c  /* 8 bits */

#define PCI_CFG_EHCI_BAR_CLASS_BASE  0xc
#define PCI_CFG_EHCI_BAR_CLASS_SUB   0x3
#define PCI_CFG_EHCI_BAR_CLASS_PI    0x20

#define pci_class_is_ehci(_cc_)  ((_cc_).class.base == PCI_CFG_EHCI_BAR_CLASS_BASE && \
				  (_cc_).class.sub  == PCI_CFG_EHCI_BAR_CLASS_SUB  && \
				  (_cc_).class.pi   == PCI_CFG_EHCI_BAR_CLASS_PI)

typedef union pci_config_space_ehci_bar
{
   struct
   {
      uint32_t  zero:1;
      uint32_t  type:2; /* 0: 32 bits, 2: 64 bits*/
      uint32_t  rsvd:5;
      uint32_t  addr:24;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_ehci_bar_t;

/*
** Specific ehci pci config space
** these registers start at extended
** capabilities (hccparams.eecp)
*/
#define PCI_CFG_USB_LEGSUP_OFFSET    0
#define PCI_CFG_USB_LEGCTL_OFFSET    4

typedef union pci_config_space_usb_legacy_support
{
   struct
   {
      uint32_t  cap_id:8;
      uint32_t  cap_next:8;
      uint32_t  bios_sem:1;
      uint32_t  rsv0:7;
      uint32_t  os_sem:1;
      uint32_t  rsv1:7;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_usblegsup_t;

typedef union pci_config_space_usb_legacy_control_status
{
   struct
   {
      uint32_t smi_enbl:1;
      uint32_t smi_err_enbl:1;
      uint32_t smi_pc_enbl:1;
      uint32_t smi_flr_enbl:1;
      uint32_t smi_hse_enbl:1;
      uint32_t smi_aa_enbl:1;
      uint32_t rsv0:7;
      uint32_t smi_os_own_enbl:1;
      uint32_t smi_pci_cmd_enbl:1;
      uint32_t smi_bar_enbl:1;
      uint32_t smi_usb_cplt:1;
      uint32_t smi_usb_err:1;
      uint32_t smi_pcd:1;
      uint32_t smi_flr:1;
      uint32_t smi_hse:1;
      uint32_t smi_aa:1;
      uint32_t rsv1:7;
      uint32_t smi_os_own_chg:1;
      uint32_t smi_pci_cmd:1;
      uint32_t smi_bar:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_usblegctl_t;

/*
** pci capability for ehci debug port
*/
#define PCI_DBGP_CAP_ID  0xa

typedef union pci_config_space_debug_port_capability
{
   struct
   {
      uint32_t   id:8;
      uint32_t   next:8;
      uint32_t   offset:13; /* offset into base addr register */
      uint32_t   nr:3;      /* bar number [1-6], memory bar only */

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) pci_cfg_dbg_cap_t;


/*
** Functions
*/
struct ehci_debug_port_info;
typedef int (*pci_search_t)(pci_cfg_val_t*);

void pci_cfg_dbgp(struct ehci_debug_port_info*);
void pci_cfg_dbgp_vendor_specific(struct ehci_debug_port_info*);
void pci_cfg_dbgp_nvidia(struct ehci_debug_port_info*);

int  pci_search(pci_search_t, pci_cfg_val_t*);
int  pci_check_cap(pci_cfg_val_t*);
int  pci_read_bar(pci_cfg_val_t*, uint32_t, uint8_t);

#define pci_read_mm_bar(pci,nr)  pci_read_bar(pci,nr,0)
#define pci_read_io_bar(pci,nr)  pci_read_bar(pci,nr,1)


#endif

