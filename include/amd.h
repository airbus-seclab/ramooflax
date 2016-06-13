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
#ifndef __AMD_CPU_H__
#define __AMD_CPU_H__

#define HYPERCALL_INSN_SZ        3

/*
** We enabling interrupts we assume
** that they are off
** We disabling interrupts we assume
** that they are on
**
** stgi re-enables normal operation that is
** if eflags.if=0 then no interrupts are taken
**
** sti enables interrupts after execution of
** next insn if eflags.if=0 when sti is executed
**
*/
#ifndef __INIT__
#define force_interrupts_on()    asm volatile( "sti ; stgi" )
#define force_interrupts_off()   asm volatile( "clgi ; cli" )
#define halt()                   asm volatile( "cli ; stgi ; sti ; hlt ; clgi ; cli" )
#else
#define force_interrupts_on()    asm volatile( "sti" )
#define force_interrupts_off()   asm volatile( "cli" )
#endif
/*
** We only allow one SMI to be taken
** at a time into the vmm
** so we clear GIF right after returning from SMM
*/
#define smi_preempt()            asm volatile( "stgi ; clgi" )

/*
** Hyper Transport Function
*/
#define AMD_CONFIG_ADDR_HTT_FUNC       0

/* Node ID Register */
#define AMD_CONFIG_ADDR_HTT_NODE_REG   ((0x60/4)<<2)

typedef union amd_conf_addr_htt_node_id
{
   struct
   {
      uint32_t  node_id:3;
      uint32_t  rsrvd0:1;
      uint32_t  node_nr:3;
      uint32_t  rsrvd1:1;
      uint32_t  sb_node:3;
      uint32_t  rsrvd2:1;
      uint32_t  lk_node:3;
      uint32_t  rsrvd3:1;
      uint32_t  cpu_cnt:4;
      uint32_t  rsrvd4:12;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) amd_conf_addr_htt_node_id_t;

/* Hyper Transport Transaction Control Register */
#define AMD_CONFIG_ADDR_HTT_CTRL_REG    ((0x68/4)<<2)

typedef union amd_conf_addr_htt_ctrl_reg
{
   struct
   {
      uint32_t   dis_rdbp:1;
      uint32_t   dis_rddwp:1;
      uint32_t   dis_wrbp:1;
      uint32_t   dis_wrdwp:1;
      uint32_t   dis_mts:1;
      uint32_t   cpu1_en:1;
      uint32_t   cpu_req:1;
      uint32_t   cpu_rsp:1;
      uint32_t   dis_pmemc:1;
      uint32_t   dis_rpmemc:1;
      uint32_t   dis_fillp:1;
      uint32_t   rsp_passw:1;
      uint32_t   chg_isoc:1;
      uint32_t   buf_rel_pri:2;
      uint32_t   limit_cldt_cfg:1;
      uint32_t   lint_en:1;
      uint32_t   apic_ext_brd:1;
      uint32_t   apic_ext_id:1;
      uint32_t   apic_ext_spur:1;
      uint32_t   seq_id:1;
      uint32_t   dwn_req:2;
      uint32_t   rsrvd0:1;
      uint32_t   med_prio_bypass:2;
      uint32_t   hi_prio_bypass:2;
      uint32_t   en_rd_prio:1;
      uint32_t   dis_wr_prio:1;
      uint32_t   dis_isoc_wr_hi_prio:1;
      uint32_t   dis_isoc_wr_med_prio:1;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) amd_conf_addr_htt_ctrl_reg_t;

/*
** Miscellaneous Control
*/

/* North Bridge Capabilities
*/
#define AMD_CONFIG_ADDR_MISC_FUNC       3
#define AMD_CONFIG_ADDR_MISC_NORTH_REG  ((0xe8/4)<<2)

typedef union amd_conf_addr_misc_north_cap
{
   struct
   {
      uint32_t   cap_128:1;
      uint32_t   cap_mp:1;
      uint32_t   cap_big_mp:1;
      uint32_t   cap_ecc:1;
      uint32_t   cap_ecc_kill:1;
      uint32_t   dram_freq:2;
      uint32_t   rsrvd0:1;
      uint32_t   cap_mem_ctl:1;
      uint32_t   rsrvd1:3;
      uint32_t   cap_cmp:2;
      uint32_t   rsrvd2:18;

   } __attribute__((packed));

   raw32_t;

} __attribute__((packed)) amd_conf_addr_misc_north_cap_t;


#endif
