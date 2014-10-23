/*
** Copyright (C) 2014 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#ifndef __E1000_H__
#define __E1000_H__

#include <types.h>

/*
** Intel 82545/EM memory registers
*/
typedef union e1k_device_control_register
{
   struct
   {
      uint32_t   fd:1;
      uint32_t   rsv0:2;
      uint32_t   lrst:1;
      uint32_t   rsv1:1;
      uint32_t   asde:1;
      uint32_t   slu:1;
      uint32_t   ilos:1;
      uint32_t   speed:2;
      uint32_t   rsv2:1;
      uint32_t   frcspd:1;
      uint32_t   frcdplx:1;
      uint32_t   rsv3:5;
      uint32_t   sdp0_data:1;
      uint32_t   sdp1_data:1;
      uint32_t   advd3wuc:1;
      uint32_t   en_phy_pwr_mgmt:1;
      uint32_t   sdp0_iodir:1;
      uint32_t   sdp1_iodir:1;
      uint32_t   rsv4:2;
      uint32_t   rst:1;
      uint32_t   rfce:1;
      uint32_t   tfce:1;
      uint32_t   rsv5:1;
      uint32_t   vme:1;
      uint32_t   phy_rst:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_ctl_reg_t;

typedef union e1k_device_status_register
{
   struct
   {
      uint32_t   fd:1;
      uint32_t   lu:1;
      uint32_t   fid:2;
      uint32_t   txoff:1;
      uint32_t   tbimode:1;
      uint32_t   speed:2;
      uint32_t   asdv:2;
      uint32_t   rsv0:1;
      uint32_t   pci66:1;
      uint32_t   bus64:1;
      uint32_t   pcix:1;
      uint32_t   pcixspd:2;
      uint32_t   rsv1:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_sts_reg_t;

typedef union e1k_interrupt_cause_register
{
   struct
   {
      uint32_t   txdw:1;
      uint32_t   txqe:1;
      uint32_t   lsc:1;
      uint32_t   rxseq:1;
      uint32_t   rxdmt0:1;
      uint32_t   rsv0:1;
      uint32_t   rxo:1;
      uint32_t   rxt0:1;
      uint32_t   rsv1:1;
      uint32_t   mdac:1;
      uint32_t   rxcfg:1;
      uint32_t   rsv2:1;
      uint32_t   phyint:1;
      uint32_t   gpi_sdp6:1;
      uint32_t   gpi_sdp7:2;
      uint32_t   txdlow:1;
      uint32_t   srpd:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_ic_reg_t;

typedef union e1k_interrupt_throttling_register
{
   struct
   {
      uint32_t   interval:16;
   }

   raw32_t;

} __attribute__((packed)) e1k_ith_reg_t;

#define e1k_rctl_sz_2k(ctl)  ({(ctl)->bsize = 0; (ctl)->bsex = 0;})
#define e1k_rctl_sz_1k(ctl)  ({(ctl)->bsize = 1; (ctl)->bsex = 0;})
#define e1k_rctl_sz_512(ctl) ({(ctl)->bsize = 2; (ctl)->bsex = 0;})
#define e1k_rctl_sz_256(ctl) ({(ctl)->bsize = 3; (ctl)->bsex = 0;})
#define e1k_rctl_sz_16k(ctl) ({(ctl)->bsize = 1; (ctl)->bsex = 1;})
#define e1k_rctl_sz_8k(ctl)  ({(ctl)->bsize = 2; (ctl)->bsex = 1;})
#define e1k_rctl_sz_4k(ctl)  ({(ctl)->bsize = 3; (ctl)->bsex = 1;})

#define E1K_RCTL_MIN_TH_2   0
#define E1K_RCTL_MIN_TH_4   1
#define E1K_RCTL_MIN_TH_8   2

typedef union e1k_receive_control_register
{
   struct
   {
      uint32_t   rsv0:1;
      uint32_t   en:1;
      uint32_t   sbp:1;
      uint32_t   upe:1;
      uint32_t   mpe:1;
      uint32_t   lpe:1;
      uint32_t   lbm:2;
      uint32_t   rdmts:2;
      uint32_t   rsv1:2;
      uint32_t   mo:2;
      uint32_t   rsv2:1;
      uint32_t   bam:1;
      uint32_t   bsize:2;
      uint32_t   vfe:1;
      uint32_t   cfien:1;
      uint32_t   cfi:1;
      uint32_t   rsv3:1;
      uint32_t   dpf:1;
      uint32_t   pmcf:1;
      uint32_t   rsv4:1;
      uint32_t   bsex:1;
      uint32_t   secrc:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_rctl_reg_t;

typedef union e1k_transmit_control_register
{
   struct
   {
      uint32_t   rsv0:1;
      uint32_t   en:1;
      uint32_t   rsv1:1;
      uint32_t   psp:1;
      uint32_t   ct:8;
      uint32_t   cold:10;
      uint32_t   swxoff:1;
      uint32_t   rsv2:1;
      uint32_t   rtlc:1;
      uint32_t   nrtu:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_tctl_reg_t;

typedef union e1k_transmit_ipg_register
{
   struct
   {
      uint32_t   ipgt:10;
      uint32_t   ipgr1:10;
      uint32_t   ipgr2:10;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_tipg_reg_t;

typedef union e1k_rxtx_descriptor_base_address_low
{
   struct
   {
      uint32_t   zero:4;
      uint32_t   addr:28;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_rxtx_dbal_reg_t;

typedef union e1k_rxtx_descriptor_length
{
   struct
   {
      uint32_t   zero:7;
      uint32_t   len:13;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_rxtx_dlen_reg_t;

typedef union e1k_rxtx_descriptor_head
{
   struct
   {
      uint32_t   head:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_rxtx_dhead_reg_t;

typedef union e1k_rxtx_descriptor_tail
{
   struct
   {
      uint32_t   tail:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_rxtx_dtail_reg_t;

/*
** RX/TX descriptors
*/
typedef union e1k_receive_descriptor_status
{
   struct
   {
      uint8_t   dd:1;
      uint8_t   eop:1;
      uint8_t   ixsm:1;
      uint8_t   vp:1;
      uint8_t   rsv:1;
      uint8_t   tcpcs:1;
      uint8_t   ipcs:1;
      uint8_t   pif:1;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) e1k_rdesc_sts_t;

typedef union e1k_receive_descriptor_error
{
   struct
   {
      uint8_t   ce:1;
      uint8_t   se:1;
      uint8_t   seq:1;
      uint8_t   rsv:1;
      uint8_t   cxe:1;
      uint8_t   tcpe:1;
      uint8_t   ipe:1;
      uint8_t   rxe:1;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) e1k_rdesc_err_t;

typedef union e1k_rxtx_descriptor_special
{
   struct
   {
      uint16_t   vlan:12;
      uint16_t   cfi:1;
      uint16_t   pri:3;

   } __attribute__((packed));

   uint16_t raw;

} __attribute__((packed)) e1k_rxtx_desc_spc_t;

typedef struct e1k_receive_descriptor
{
   uint64_t             addr;
   uint16_t             len;
   uint16_t             sum;
   e1k_rdesc_sts_t      sts;
   e1k_rdesc_err_t      err;
   e1k_rxtx_desc_spc_t  spc;

} __attribute__((packed, aligned(16))) e1k_rdesc_t;

typedef union e1k_transmit_descriptor_command
{
   struct
   {
      uint8_t   eop:1;
      uint8_t   ifcs:1;
      uint8_t   ic:1;
      uint8_t   rs:1;
      uint8_t   rsv0:1;
      uint8_t   dext:1;
      uint8_t   vle:1;
      uint8_t   ide:1;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) e1k_tdesc_cmd_t;

typedef union e1k_transmit_descriptor_status
{
   struct
   {
      uint8_t   dd:1;
      uint8_t   ec:1;
      uint8_t   lc:1;

      uint8_t   rsv:5;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) e1k_tdesc_sts_t;

typedef struct e1k_transmit_descriptor
{
   uint64_t             addr;
   uint16_t             len;
   uint8_t              cso;
   e1k_tdesc_cmd_t      cmd;
   e1k_tdesc_sts_t      sts;
   uint8_t              css;
   e1k_rxtx_desc_spc_t  spc;

} __attribute__((packed, aligned(16))) e1k_tdesc_t;

/*
** Receive Address / MTA filtering
*/
typedef union e1k_receive_address
{
   struct
   {
      raw32_t    ral;
      uint16_t   rah;

      uint64_t   as:2;
      uint64_t   rsv:13;
      uint64_t   av:1;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) e1k_ra_reg_t;

/*
** EEPROM access
*/
#define E1000_EE_MACADDR    0

typedef union e1k_eeprom_read
{
   struct
   {
      uint32_t   start:1;
      uint32_t   rsv0:3;
      uint32_t   done:1;
      uint32_t   rsv1:3;
      uint32_t   addr:8;
      uint32_t   data:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) e1k_eerd_reg_t;

/*
** Convenient structure
*/
typedef struct e1k_rxtx_registers
{
   volatile e1k_rxtx_dbal_reg_t  *bal;
   volatile raw32_t              *bah;
   volatile e1k_rxtx_dlen_reg_t  *dl;
   volatile e1k_rxtx_dhead_reg_t *dh;
   volatile e1k_rxtx_dtail_reg_t *dt;

   e1k_rxtx_dtail_reg_t tail;  /* cache cause unreliable */

} __attribute__((packed)) e1k_rxtx_reg_t;

typedef struct e1k_rx_registers
{
   volatile e1k_rctl_reg_t *ctl;
   volatile e1k_rxtx_reg_t;

} __attribute__((packed)) e1k_rx_reg_t;

typedef struct e1k_tx_registers
{
   volatile e1k_tctl_reg_t *ctl;
   volatile e1k_tipg_reg_t *ipg;
   volatile e1k_rxtx_reg_t;

} __attribute__((packed)) e1k_tx_reg_t;

#define RX_DESC_NR  16
#define RX_BUFF_SZ  2048

#define TX_DESC_NR  16
#define TX_BUFF_SZ  2048

typedef struct e1k_memory_data
{
   volatile e1k_rdesc_t *rdesc;
   volatile e1k_tdesc_t *tdesc;
   loc_t                rx_dma;
   loc_t                tx_dma;

} __attribute__((packed)) e1k_mem_t;

typedef struct e1k_info
{
   volatile loc_t          base;
   volatile e1k_ctl_reg_t  *ctl;
   volatile e1k_sts_reg_t  *sts;

   volatile uint32_t       *fcal, *fcah, *fct, *fcttv;

   volatile e1k_ic_reg_t   *icr;
   volatile e1k_ic_reg_t   *ics;
   volatile e1k_ic_reg_t   *ims;
   volatile e1k_ic_reg_t   *imc;

   volatile e1k_rx_reg_t    rx;
   volatile e1k_tx_reg_t    tx;

   volatile e1k_eerd_reg_t  *eerd;

   volatile uint32_t        *mta;
   volatile e1k_ra_reg_t    *ra;

   e1k_mem_t                mem;

} __attribute__((packed)) e1k_info_t;

/*
** Functions
*/
struct net_info;

#ifdef __INIT__
void     e1k_init(struct net_info*);

size_t   e1k_mem_size();
offset_t e1k_mem_init(e1k_info_t*, offset_t);
void     e1k_rx_init(e1k_info_t*);
void     e1k_tx_init(e1k_info_t*);
#endif

void     e1k_rx_on(struct net_info*);
void     e1k_tx_on(struct net_info*);
void     e1k_send_pkt(struct net_info*, loc_t, size_t);
size_t   e1k_recv_pkt(struct net_info*, loc_t, size_t);

offset_t e1k_tx_get_pktbuf(e1k_info_t*);

#endif
