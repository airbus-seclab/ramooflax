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
#ifndef __PCI_DBGP_H__
#define __PCI_DBGP_H__

#include <types.h>
#include <pci.h>

/*
** Specific ehci pci config space
*/
#define PCI_CFG_EHCI_INT_LINE_OFFSET 0x3c  /* 8 bits */

#define PCI_CFG_EHCI_BAR_CLASS_BASE  0xc
#define PCI_CFG_EHCI_BAR_CLASS_SUB   0x3
#define PCI_CFG_EHCI_BAR_CLASS_PI    0x20

#define pci_class_is_ehci(_cc_)						\
   ((_cc_).class.base == PCI_CFG_EHCI_BAR_CLASS_BASE &&			\
    (_cc_).class.sub  == PCI_CFG_EHCI_BAR_CLASS_SUB  &&			\
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

void pci_cfg_dbgp(struct ehci_debug_port_info*);
void pci_cfg_dbgp_vendor_specific(struct ehci_debug_port_info*);
void pci_cfg_dbgp_nvidia(struct ehci_debug_port_info*);

#endif
