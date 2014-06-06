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
#ifndef __MP_H__
#define __MP_H__

#include <types.h>

/*
** Intel Multi Processor Specification v1.4
*/
#define MP_PTR_SIG              0x5f504d5f  /* _MP_ */

#define MP_PTR_FEAT_0_MP        0
#define MP_PTR_FEAT_1_IMCR      (1<<7)

typedef struct mp_floating_pointer_structure
{
   uint32_t  sig;
   uint32_t  addr;
   uint8_t   len;
   uint8_t   rev;
   uint8_t   checksum;
   uint8_t   features[5];

} __attribute__((packed)) mp_ptr_t;

#define MP_TBL_SIG              0x504d4350  /* PCMP */
#define MP_TBL_ENTRY_PROC       0
#define MP_TBL_ENTRY_BUS        1
#define MP_TBL_ENTRY_IOAPIC     2
#define MP_TBL_ENTRY_IOINT      3
#define MP_TBL_ENTRY_LINT       4
#define MP_TBL_ENTRY_SMAP       128
#define MP_TBL_ENTRY_BUS_DESC   129
#define MP_TBL_ENTRY_BUS_COMPAT 130

#define MP_TBL_ENTRY_SZ_PROC   20
#define MP_TBL_ENTRY_SZ_OTHER   8

typedef struct mp_table
{
   uint32_t  sig;
   uint16_t  len;
   uint8_t   rev;
   uint8_t   checksum;
   uint8_t   oem_id[8];
   uint8_t   prd_id[12];
   uint32_t  oem_addr;
   uint16_t  oem_len;
   uint16_t  count;
   uint32_t  lapic_addr;
   uint16_t  ext_len;
   uint8_t   ext_checksum;

} __attribute__((packed)) mp_table_t;

/*
** Processor entry
*/
typedef union mp_table_processor_entry_flags
{
   struct
   {
      uint8_t en:1;    /* enabled */
      uint8_t bp:1;    /* bootstrap proc */
      uint8_t rsvd0:6;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) mp_tbl_proc_flags_t;

typedef struct mp_table_processor_entry
{
   uint8_t             type;
   uint8_t             lapic_id;
   uint8_t             lapic_ver;
   mp_tbl_proc_flags_t flags;
   uint32_t            sig;
   uint32_t            features;
   uint64_t            rsvd;

} __attribute__((packed)) mp_tbl_proc_t;

/*
** IO Apic entry
*/
typedef union mp_table_ioapic_entry_flags
{
   struct
   {
      uint8_t en:1;    /* enabled */
      uint8_t rsvd0:7;

   } __attribute__((packed));

   uint8_t raw;

} __attribute__((packed)) mp_tbl_ioapic_flags_t;

typedef struct mp_table_ioapic_entry
{
   uint8_t               type;
   uint8_t               ioapic_id;
   uint8_t               ioapic_ver;
   mp_tbl_ioapic_flags_t flags;
   uint32_t              addr;

} __attribute__((packed)) mp_tbl_ioapic_t;

/*
** IO/Local Interrupt entries
*/
#define MP_TBL_INTR_TYPE_INT    0
#define MP_TBL_INTR_TYPE_NMI    1
#define MP_TBL_INTR_TYPE_SMI    2
#define MP_TBL_INTR_TYPE_EXT    3

typedef union mp_table_intr_entry_flags
{
   struct
   {
      uint8_t  po:1;      /* polarity */
      uint8_t  el:1;      /* trigger mode */
      uint16_t rsvd0:13;

   } __attribute__((packed));

   uint16_t raw;

} __attribute__((packed)) mp_tbl_intr_flags_t;

typedef struct mp_table_intr_entry
{
   uint8_t             type;           /* MP_TBL_ENTRY_IOINT or MP_TBL_ENTRY_LINT */
   uint8_t             intr_type;
   mp_tbl_intr_flags_t flags;
   uint8_t             src_bus_id;
   uint8_t             src_bus_irq;
   uint8_t             dstc_apic_id;    /* local or IO apic */
   uint8_t             dstc_apic_intr;

} __attribute__((packed)) mp_tbl_intr_t;


/*
** Functions
*/
#ifdef __INIT__
void  mp_init();
#endif

#endif
