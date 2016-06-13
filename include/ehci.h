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
#ifndef __EHCI_H__
#define __EHCI_H__

#include <types.h>
#include <pci_dbgp.h>

/*
** PID (with complement)
*/
#define EHCI_TOKEN_PID_SETUP    0x2d
#define EHCI_TOKEN_PID_IN       0x69
#define EHCI_TOKEN_PID_OUT      0xe1

#define EHCI_DATA_PID_1         0x4b
#define EHCI_DATA_PID_0         0xc3

#define EHCI_STATUS_PID_STALL   0x1e
#define EHCI_STATUS_PID_NAK     0x5a
#define EHCI_STATUS_PID_NYET    0x96
#define EHCI_STATUS_PID_ACK     0xd2

#define ehci_pid_swap(_p_)      ((_p_) ^= 0x88)

#define ehci_pid_valid(_p_)                                             \
   ((_p_) == EHCI_STATUS_PID_ACK || (_p_) == EHCI_STATUS_PID_NYET ||    \
    (_p_) == EHCI_DATA_PID_0     || (_p_) == EHCI_DATA_PID_1)

/*
** PORTSC transitions
*/
#define EHCI_PSC_DBC2DD         0 /* debounce to disabled disconnected */
#define EHCI_PSC_DBC2RST        1 /* debounce to high/full-speed, need reset */

#define EHCI_PSC_RST2DD         0 /* reset to disabled disconnected */
#define EHCI_PSC_RST2HS         1 /* reset to high-speed */

/*
** USB standard request codes
*/
#define USB_REQ_GET_STATUS                       0
#define USB_REQ_CLEAR_FEATURE                    1
#define USB_REQ_SET_FEATURE                      3
#define USB_REQ_SET_ADDRESS                      5
#define USB_REQ_GET_DESCRIPTOR                   6
#define USB_REQ_SET_DESCRIPTOR                   7
#define USB_REQ_GET_CONFIGURATION                8
#define USB_REQ_SET_CONFIGURATION                9
#define USB_REQ_GET_INTERFACE                   10
#define USB_REQ_SET_INTERFACE                   11
#define USB_REQ_SYNCH_FRAME                     12

/*
** USB descriptor types
*/
#define USB_DESC_TYPE_DEVICE                     1
#define USB_DESC_TYPE_CONFIGURATION              2
#define USB_DESC_TYPE_STRING                     3
#define USB_DESC_TYPE_INTERFACE                  4
#define USB_DESC_TYPE_ENDPOINT                   5
#define USB_DESC_TYPE_DEVICE_QUALIFIER           6
#define USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION  7
#define USB_DESC_TYPE_INTERFACE_POWER            8
#define USB_DESC_TYPE_OTG                        9
#define USB_DESC_TYPE_DEBUG                     10

/*
** USB feature types
*/
#define USB_FEAT_DEBUG_MODE                       6

/*
** USB request types
*/
#define USB_REQTYPE_RECIPIENT_DEVICE             0
#define USB_REQTYPE_RECIPIENT_INTERFACE          1
#define USB_REQTYPE_RECIPIENT_ENDPOINT           2
#define USB_REQTYPE_RECIPIENT_OTHER              3

#define USB_REQTYPE_TYPE_STANDARD                0
#define USB_REQTYPE_TYPE_CLASS                   1
#define USB_REQTYPE_TYPE_VENDOR                  2

#define USB_REQTYPE_DTD_H2D                      0
#define USB_REQTYPE_DTD_D2H                      1

typedef union usb_request_type
{
   struct
   {
      uint8_t recipient:5;
      uint8_t type:2;
      uint8_t dtd:1;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) usb_req_type_t;

/*
** Usb device request
*/
typedef union usb_device_request
{
   struct
   {
      usb_req_type_t req_type;
      uint64_t       request:8;
      uint64_t       value:16;
      uint64_t       index:16;
      uint64_t       len:16;

   } __attribute__((packed));

   raw64_t;

} __attribute__((packed)) usb_dev_req_t;

typedef union usb_debug_device_descriptor
{
   struct
   {
      uint32_t   len:8;
      uint32_t   type:8;
      uint32_t   ep_in:8;
      uint32_t   ep_out:8;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) usb_dbg_desc_t;

/*
** Memory mapped ehci registers
*/
typedef union ehci_hcsparams_register
{
   struct
   {
      uint32_t  n_ports:4;
      uint32_t  ppc:1;
      uint32_t  rsvd0:2;
      uint32_t  prr:1;
      uint32_t  n_pcc:4;
      uint32_t  n_cc:4;
      uint32_t  p_indic:1;
      uint32_t  rsvd1:3;
      uint32_t  dbg_nr:4;
      uint32_t  rsvd2:8;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_hcsparams_reg_t;

typedef union ehci_hccparams_register
{
   struct
   {
      uint32_t   cap64:1;
      uint32_t   prog_frame:1;
      uint32_t   sched_park:1;
      uint32_t   rsvd0:1;
      uint32_t   sched_thrsh:4;
      uint32_t   eecp:8;
      uint32_t   rsvd1:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_hccparams_reg_t;

typedef union ehci_usb_command_register
{
   struct
   {
      uint32_t  run:1;
      uint32_t  rst:1;
      uint32_t  frame_list_sz:2;
      uint32_t  p_sched_enbl:1;
      uint32_t  a_sched_enbl:1;
      uint32_t  doorbell:1;
      uint32_t  light_rst:1;
      uint32_t  sched_park_cnt:2;
      uint32_t  rsvd0:1;
      uint32_t  sched_park_enbl:1;
      uint32_t  rsvd1:4;
      uint32_t  int_thrsh:8;
      uint32_t  rsvd2:8;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_usbcmd_reg_t;

typedef union ehci_usb_status_register
{
   struct
   {
      uint32_t   usb_int:1;
      uint32_t   err_int:1;
      uint32_t   port_change:1;
      uint32_t   frame_list_rollover:1;
      uint32_t   sys_err:1;
      uint32_t   int_advance:1;
      uint32_t   rsvd1:6;
      uint32_t   hlt:1;
      uint32_t   reclame:1;
      uint32_t   periodic_sched_status:1;
      uint32_t   sched_status:1;
      uint32_t   rsvd0:16;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_usbsts_reg_t;

typedef union ehci_usb_interrupt_register
{
   struct
   {
      uint32_t  usb:1;
      uint32_t  usb_err:1;
      uint32_t  port_change:1;
      uint32_t  frame_list_rollover:1;
      uint32_t  sys_err:1;
      uint32_t  advance:1;
      uint32_t  rsvd:26;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_usbint_reg_t;

typedef union ehci_frame_index_register
{
   struct
   {
      uint32_t frame_index:14;
      uint32_t rsvd:18;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_frindx_reg_t;

typedef union ehci_periodic_frame_list_base_address_register
{
   struct
   {
      uint32_t rsvd:12;
      uint32_t base:20;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_pbase_reg_t;

typedef union ehci_async_list_base_address_register
{
   struct
   {
      uint32_t  rsvd:5;
      uint32_t  lpl:27;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_abase_reg_t;

typedef union ehci_config_flag_register
{
   struct
   {
      uint32_t cf:1;
      uint32_t rsvd:31;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_cfgflg_reg_t;

typedef union ehci_portsc_register
{
   struct
   {
      uint32_t  conn:1;
      uint32_t  conn_chg:1;
      uint32_t  enbl:1;
      uint32_t  enbl_chg:1;
      uint32_t  over:1;
      uint32_t  over_chg:1;
      uint32_t  force_rsm:1;
      uint32_t  suspend:1;
      uint32_t  reset:1;
      uint32_t  rsv0:1;
      uint32_t  line_sts:2;
      uint32_t  pow:1;
      uint32_t  own:1;
      uint32_t  ctrl_ind:2;
      uint32_t  ctrl_tst:4;
      uint32_t  woc:1;
      uint32_t  wod:1;
      uint32_t  woo:1;
      uint32_t  rsv1:9;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_portsc_reg_t;

typedef union ehci_debug_port_control_status_register
{
   struct
   {
      uint32_t  dlen:4;
      uint32_t  wr:1;
      uint32_t  go:1;
      uint32_t  err:1;
      uint32_t  excp:3;
      uint32_t  used:1;
      uint32_t  rsv0:5;
      uint32_t  done:1;
      uint32_t  rsv1:11;
      uint32_t  enbl:1;
      uint32_t  rsv2:1;
      uint32_t  ownd:1;
      uint32_t  rsv3:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_dbgp_ctrl_reg_t;

typedef union ehci_debug_port_pid_register
{
   struct
   {
      uint32_t tkn:8;
      uint32_t snd:8;
      uint32_t rcv:8;
      uint32_t rsv:8;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_dbgp_pid_reg_t;

typedef union ehci_debug_port_device_address_register
{
   struct
   {
      uint32_t  ep:4;
      uint32_t  rsv0:4;
      uint32_t  addr:7;
      uint32_t  rsv1:17;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) ehci_dbgp_dev_reg_t;

/*
** These registers are located at pci config space BAR #1
*/
typedef struct ehci_host_controller_capability_registers
{
   uint8_t              length;
   uint8_t              rsvd;
   uint16_t             version;
   ehci_hcsparams_reg_t hcs;
   ehci_hccparams_reg_t hcc;
   raw64_t              hcsp;

} __attribute__((packed)) ehci_host_cap_reg_t;

/*
** These registers area located at ehci host cap + cap length
*/
typedef struct ehci_host_controller_operational_registers
{
   ehci_usbcmd_reg_t    usbcmd;
   ehci_usbsts_reg_t    usbsts;
   ehci_usbint_reg_t    usbint;
   ehci_frindx_reg_t    frindx;
   raw32_t              ctrldssegment;
   ehci_pbase_reg_t     periodic_base;
   ehci_abase_reg_t     async_base;
   uint8_t              reserved[36];
   ehci_cfgflg_reg_t    cfgflg;

   ehci_portsc_reg_t    portsc[0];

} __attribute__((packed)) ehci_host_op_reg_t;

/*
** These registers are located using debug cap BAR and offset
*/
typedef struct ehci_debug_port_registers
{
   ehci_dbgp_ctrl_reg_t ctrl;
   ehci_dbgp_pid_reg_t  pid;
   raw64_t              data;
   ehci_dbgp_dev_reg_t  dev;

} __attribute__((packed)) ehci_dbgp_reg_t;

/*
** Convenient structure
*/
typedef struct ehci_debug_port_info
{
   volatile ehci_host_cap_reg_t  *ehci_cap;
   volatile ehci_host_op_reg_t   *ehci_opr;
   volatile ehci_portsc_reg_t    *ehci_psc;
   volatile ehci_dbgp_reg_t      *ehci_dbg;

   pci_cfg_val_t                 pci;
   usb_dbg_desc_t                desc;
   uint8_t                       addr;
   uint8_t                       port;
   size_t                        fails;

} __attribute__((packed)) dbgp_info_t;

/*
** Functions
*/
#define DBGP_RETRY      1000
#define DBGP_FAIL_TRSH     5
#define DBGP_DEV_ADDR    127
#define DBGP_PKT_SZ        8
#define DBGP_INVALID       0

#define DBGP_EMIT_FAIL     0
#define DBGP_EMIT_OK       1

#ifdef __INIT__
void ehci_init();
void ehci_full_init(dbgp_info_t*);
void ehci_fast_init(dbgp_info_t*);
void ehci_controller_preinit(dbgp_info_t*);
void ehci_controller_postinit(dbgp_info_t*);

void ehci_acquire(dbgp_info_t*);
void ehci_release(dbgp_info_t*);
void ehci_remove_smi(dbgp_info_t*);
void ehci_reset(dbgp_info_t*);
#endif

void ehci_dbgp_init(dbgp_info_t*);
void ehci_dbgp_full_init(dbgp_info_t*);
void ehci_dbgp_fast_init(dbgp_info_t*);
void ehci_dbgp_stealth_reinit(dbgp_info_t*);

void ehci_setup(dbgp_info_t*);
void ehci_detect(dbgp_info_t*, int);
void ehci_ports_identify(dbgp_info_t*);

void dbgp_enable(dbgp_info_t*);
void dbgp_own(dbgp_info_t*);
void dbgp_release(dbgp_info_t*);
void dbgp_configure(dbgp_info_t*);

void __ehci_port_own(dbgp_info_t*);
void __ehci_port_leave_disabled(dbgp_info_t*);
int  __ehci_port_debounce_from_disabled(dbgp_info_t*);
int  __ehci_port_reset_from_debounce(dbgp_info_t*);
int  __ehci_port_disabled_connected(dbgp_info_t*);

void __ehci_port_hs_active_from_reset(dbgp_info_t*);
void __ehci_port_transfert_companion(dbgp_info_t*);
void __ehci_port_reset(dbgp_info_t*);
void __ehci_port_hide(dbgp_info_t*);

void __dbgp_init(dbgp_info_t*);
void __dbgp_enable_without_ehci_cfg(dbgp_info_t*);
void __dbgp_enable_with_ehci_cfg(dbgp_info_t*);
int  __dbgp_get_descriptor(dbgp_info_t*);
int  __dbgp_set_addr(dbgp_info_t*);
int  __dbgp_set_debug_mode(dbgp_info_t*);
int  __dbgp_setup(dbgp_info_t*, usb_dev_req_t*);

void __ehci_show(dbgp_info_t*);
int  __dbgp_read(dbgp_info_t*, buffer_t*, uint8_t);
int   _dbgp_read(dbgp_info_t*, buffer_t*);
int  __dbgp_write(dbgp_info_t*, buffer_t*);
int  __dbgp_emit(dbgp_info_t*, ehci_dbgp_reg_t*, buffer_t*);

typedef int (*dbgp_rw_t)(dbgp_info_t*, buffer_t*);

size_t dbgp_write(uint8_t*, size_t);
size_t dbgp_read(uint8_t*, size_t);
#endif
