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
#include <ctrl.h>
#include <gdb.h>
#include <insn.h>
#include <cr.h>
#include <paging.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void gdb_vmm_rd_all_sysregs(uint8_t __unused__ *data, size_t __unused__ len)
{
   size_t rlen = sizeof(uint64_t)*2;

   gdb_add_number(__cr0.raw,            rlen, 1);
   gdb_add_number(__cr2.raw,            rlen, 1);
   gdb_add_number(__cr3.raw,            rlen, 1);
   gdb_add_number(__cr4.raw,            rlen, 1);
   gdb_add_number(get_dr0(),            rlen, 1);
   gdb_add_number(get_dr1(),            rlen, 1);
   gdb_add_number(get_dr2(),            rlen, 1);
   gdb_add_number(get_dr3(),            rlen, 1);
   gdb_add_number(__dr6.raw,            rlen, 1);
   gdb_add_number(__dr7.raw,            rlen, 1);
   gdb_add_number(__dbgctl.raw,         rlen, 1);
   gdb_add_number(__efer.raw,           rlen, 1);
   gdb_add_number(__cs.base_addr.raw,   rlen, 1);
   gdb_add_number(__ss.base_addr.raw,   rlen, 1);
   gdb_add_number(__ds.base_addr.raw,   rlen, 1);
   gdb_add_number(__es.base_addr.raw,   rlen, 1);
   gdb_add_number(__fs.base_addr.raw,   rlen, 1);
   gdb_add_number(__gs.base_addr.raw,   rlen, 1);
   gdb_add_number(__gdtr.base_addr.raw, rlen, 1);
   gdb_add_number(__idtr.base_addr.raw, rlen, 1);
   gdb_add_number(__ldtr.base_addr.raw, rlen, 1);
   gdb_add_number(__tr.base_addr.raw,   rlen, 1);

   gdb_send_packet();
}

static void gdb_vmm_wr_all_sysregs(uint8_t *data, size_t len)
{
   debug(GDB_CMD, "need to implement write all system registers\n");
   data = data;
   len  = len;
   gdb_unsupported();
}

static void gdb_vmm_rd_sysreg(uint8_t *data, size_t len)
{
   raw64_t *reg;
   size_t  size;

   if(!__gdb_setup_reg_op(data, len, &reg, &size, 0, 0, 1))
   {
      debug(GDB_CMD, "read sysreg failed\n");
      return;
   }

   size *= 2; /* to nibbles */
   gdb_add_number(reg->raw, size, 1);
   gdb_send_packet();
}

static void gdb_vmm_wr_sysreg(uint8_t *data, size_t len)
{
   raw64_t  *reg, value;
   size_t   size;

   if(!__gdb_setup_reg_op(data, len, &reg, &size, &value, 1, 1))
   {
      debug(GDB_CMD, "write sysreg failed\n");
      return;
   }

   if(reg == (raw64_t*)&__cr0 && __resolve_cr0_wr((cr0_reg_t*)&value) == CR_FAIL)
      goto __err;
   else if(reg == (raw64_t*)&__cr3 && __resolve_cr3_wr((cr3_reg_t*)&value) == CR_FAIL)
      goto __err;
   else if(reg == (raw64_t*)&__cr4 && __resolve_cr4_wr((cr4_reg_t*)&value) == CR_FAIL)
      goto __err;
   else
      reg->raw = value.raw;

   gdb_ok();
   return;

__err:
   debug(GDB_CMD, "invalid wr to control register\n");
   gdb_err_inv();
}

static void gdb_vmm_set_lbr(uint8_t __unused__ *data, size_t __unused__ len)
{
   __enable_lbr();
   gdb_ok();
}

static void gdb_vmm_del_lbr(uint8_t __unused__ *data, size_t __unused__ len)
{
   __disable_lbr();
   gdb_ok();
}

static void gdb_vmm_get_lbr(uint8_t __unused__ *data, size_t __unused__ len)
{
   int      i;
   size_t   rlen;
   offset_t ips[4];

   rlen = sizeof(uint64_t)*2;

   ips[0] = __lbr_from();
   ips[1] = __lbr_to();
   ips[2] = __lbr_from_excp();
   ips[3] = __lbr_to_excp();

   for(i=0 ; i<4 ; i++)
      gdb_add_number(__cs.base_addr.raw + ips[i], rlen, 0);

   gdb_send_packet();
}

static void gdb_vmm_rd_excp_mask(uint8_t __unused__ *data, size_t __unused__ len)
{
   raw64_t val;
   size_t  rlen;

   rlen = sizeof(uint32_t)*2;
   val.low = info->vmm.ctrl.usr.excp;
   gdb_add_number(val.raw, rlen, 0);
   gdb_send_packet();
}

static void gdb_vmm_wr_excp_mask(uint8_t *data, size_t len)
{
   raw64_t val;

   if(!gdb_get_number(data, len, (uint64_t*)&val.raw, 0))
   {
      gdb_nak();
      return;
   }

   info->vmm.ctrl.usr.excp = val.low;
   __update_exception_mask();
   gdb_ok();
}

static void gdb_enable_active_cr3(raw64_t cr3)
{
   info->vmm.ctrl.dbg.stored_cr3.raw = cr3.raw;
   info->vmm.ctrl.dbg.active_cr3 = &info->vmm.ctrl.dbg.stored_cr3;
   info->vmm.ctrl.dbg.status.cr3 = 1;
}

static void gdb_disable_active_cr3()
{
   info->vmm.ctrl.dbg.active_cr3 = &__cr3;
   info->vmm.ctrl.dbg.status.keep_cr3 = 0;
   info->vmm.ctrl.dbg.status.cr3 = 0;
}

static void gdb_vmm_set_active_cr3(uint8_t *data, size_t len)
{
   raw64_t val;

   if(!gdb_get_number(data, len, (uint64_t*)&val.raw, 0))
   {
      gdb_nak();
      return;
   }

   gdb_enable_active_cr3(val);
   gdb_ok();
   debug(GDB, "cr3 tracking enabled for 0x%X\n", info->vmm.ctrl.dbg.stored_cr3.raw);
}

static void gdb_vmm_del_active_cr3(uint8_t __unused__ *data, size_t __unused__ len)
{
   gdb_disable_active_cr3();
   gdb_ok();
}

static void gdb_vmm_keep_active_cr3(uint8_t __unused__ *data, size_t __unused__ len)
{
   if(info->vmm.ctrl.dbg.status.cr3)
   {
      info->vmm.ctrl.dbg.status.keep_cr3 = 1;
      gdb_ok();
      return;
   }

   gdb_err_inv();
}

static void gdb_vmm_set_affinity(uint8_t *data, size_t len)
{
   raw64_t val;

   if(!gdb_get_number(data, len, (uint64_t*)&val.raw, 0))
   {
      gdb_nak();
      return;
   }

   gdb_set_affinity(val.blow);
   gdb_ok();
   debug(GDB, "set affinity to %d\n", val.blow);
}

static int __gdb_vmm_mem_rw_parse(uint8_t *data, size_t len, loc_t *addr, size_t *sz)
{
   if(len != sizeof(offset_t)*2*2)
      goto __nak;

   len /= 2;

   if(!gdb_get_number(data, len, (uint64_t*)addr, 0))
      goto __nak;

   data += sizeof(offset_t)*2;

   if(!gdb_get_number(data, len, (uint64_t*)sz, 0))
      goto __nak;

   gdb_ok();
   gdb_wait_ack();
   return 1;

__nak:
   gdb_nak();
   return 0;
}

static void __gdb_vmm_rw_pmem(offset_t addr, size_t sz, uint8_t wr)
{
   vm_access_t access;

   access.addr = addr;
   access.len  = sz;
   access.wr   = wr;

   if(!__vm_remote_access_pmem(&access))
   {
      debug(GDB, "memory access failure\n");
      gdb_err_mem();
   }
}

static void gdb_vmm_rd_pmem(uint8_t *data, size_t len)
{
   loc_t  addr;
   size_t sz;

   if(!__gdb_vmm_mem_rw_parse(data, len, &addr, &sz))
      return;

   debug(GDB_CMD, "reading physical memory @ 0x%X sz %d\n", addr.linear, sz);
   __gdb_vmm_rw_pmem(addr.linear, sz, 1);
}

static void gdb_vmm_wr_pmem(uint8_t *data, size_t len)
{
   loc_t  addr;
   size_t sz;

   if(!__gdb_vmm_mem_rw_parse(data, len, &addr, &sz))
      return;

   debug(GDB_CMD, "writing physical memory @ 0x%X sz %d\n", addr.linear, sz);
   __gdb_vmm_rw_pmem(addr.linear, sz, 0);
}

static void gdb_vmm_rd_vmem(uint8_t *data, size_t len)
{
   loc_t  addr;
   size_t sz;

   if(!__gdb_vmm_mem_rw_parse(data, len, &addr, &sz))
      return;

   debug(GDB_CMD, "reading virtual memory @ 0x%X sz %d\n", addr.linear, sz);
   if(!__vm_send_mem(info->vmm.ctrl.dbg.active_cr3, addr.linear, sz))
   {
      debug(GDB, "memory access failure\n");
      gdb_err_mem();
      return;
   }
}

static void gdb_vmm_wr_vmem(uint8_t *data, size_t len)
{
   loc_t  addr;
   size_t sz;

   if(!__gdb_vmm_mem_rw_parse(data, len, &addr, &sz))
      return;

   debug(GDB_CMD, "writing virtual memory @ 0x%X sz %d\n", addr.linear, sz);
   if(!__vm_recv_mem(info->vmm.ctrl.dbg.active_cr3, addr.linear, sz))
   {
      debug(GDB, "memory access failure\n");
      gdb_err_mem();
      return;
   }
}

static void gdb_vmm_translate(uint8_t *data, size_t len)
{
   offset_t vaddr, paddr;
   size_t   psz, sz;

   if(!gdb_get_number(data, len, (uint64_t*)&vaddr, 0))
   {
      gdb_nak();
      return;
   }

   debug(GDB_CMD, "translating 0x%X\n", vaddr);

   if(__paging())
   {
      if(!__pg_walk(info->vmm.ctrl.dbg.active_cr3, vaddr, &paddr, &psz))
	 paddr = 0;
   }
   else
      paddr = vaddr;

   debug(GDB_CMD, "sending 0x%X\n", paddr);

   if(__lmode64())
      sz = sizeof(uint64_t)*2;
   else
      sz = sizeof(uint32_t)*2;

   gdb_add_number(paddr, sz, 0);
   gdb_send_packet();
}

static void gdb_vmm_rd_cr_rd_mask(uint8_t __unused__ *data, size_t __unused__ len)
{
   raw64_t val;
   size_t  rlen;

   rlen = sizeof(uint16_t)*2;
   val.wlow = info->vmm.ctrl.usr.cr_rd;
   gdb_add_number(val.raw, rlen, 0);
   gdb_send_packet();
}

static void gdb_vmm_wr_cr_rd_mask(uint8_t *data, size_t len)
{
   raw64_t val;

   if(!gdb_get_number(data, len, (uint64_t*)&val.raw, 0))
   {
      gdb_nak();
      return;
   }

   info->vmm.ctrl.usr.cr_rd = val.wlow & 0x1ff; /* only allow until cr8 */
   __update_cr_read_mask();
   gdb_ok();
}

static void gdb_vmm_rd_cr_wr_mask(uint8_t __unused__ *data, size_t __unused__ len)
{
   raw64_t val;
   size_t  rlen;

   rlen = sizeof(uint16_t)*2;
   val.wlow = info->vmm.ctrl.usr.cr_wr;
   gdb_add_number(val.raw, rlen, 0);
   gdb_send_packet();
}

static void gdb_vmm_wr_cr_wr_mask(uint8_t *data, size_t len)
{
   raw64_t val;

   if(!gdb_get_number(data, len, (uint64_t*)&val.raw, 0))
   {
      gdb_nak();
      return;
   }

   debug(GDB_CMD, "write cr mask 0x%x\n", val.wlow);

   info->vmm.ctrl.usr.cr_wr = val.wlow & 0x1ff; /* only allow until cr8 */
   __update_cr_write_mask();
   gdb_ok();
}

static gdb_vmm_hdl_t gdb_vmm_handlers[] = {
   gdb_vmm_rd_all_sysregs,
   gdb_vmm_wr_all_sysregs,
   gdb_vmm_rd_sysreg,
   gdb_vmm_wr_sysreg,
   gdb_vmm_set_lbr,
   gdb_vmm_del_lbr,
   gdb_vmm_get_lbr,
   gdb_vmm_rd_excp_mask,
   gdb_vmm_wr_excp_mask,
   gdb_vmm_set_active_cr3,
   gdb_vmm_del_active_cr3,
   gdb_vmm_rd_pmem,
   gdb_vmm_wr_pmem,
   gdb_vmm_rd_vmem,
   gdb_vmm_wr_vmem,
   gdb_vmm_translate,
   gdb_vmm_rd_cr_rd_mask,
   gdb_vmm_wr_cr_rd_mask,
   gdb_vmm_rd_cr_wr_mask,
   gdb_vmm_wr_cr_wr_mask,
   gdb_vmm_keep_active_cr3,
   gdb_vmm_set_affinity,
};

void gdb_cmd_vmm(uint8_t *data, size_t len)
{
   uint8_t idx = *data - 0x80;

   data++; len--;

   if(idx <= sizeof(gdb_vmm_handlers)/sizeof(gdb_vmm_hdl_t))
   {
      debug(GDB_CMD, "gdb_vmm handler call %d\n", idx);
      gdb_vmm_handlers[idx](data, len);
      return;
   }

   debug(GDB_CMD, "vmm cmd unsupported\n");
   gdb_unsupported();
}

void gdb_vmm_enable()
{
   if(!info->vmm.ctrl.dbg.status.keep_cr3)
      gdb_disable_active_cr3();
   else
   {
      debug(GDB_CMD, "gdb kept active cr3: 0x%X (0x%X <=> 0x%X)\n"
	    , info->vmm.ctrl.dbg.stored_cr3.raw
	    , info->vmm.ctrl.dbg.active_cr3
	    ,&info->vmm.ctrl.dbg.stored_cr3);
   }
}

void gdb_vmm_disable()
{
   info->vmm.ctrl.usr.excp  = 0;
   info->vmm.ctrl.usr.cr_rd = 0;
   info->vmm.ctrl.usr.cr_wr = 0;

   __update_exception_mask();
   __update_cr_read_mask();
   __update_cr_write_mask();
   __disable_lbr();

   debug(GDB_CMD, "gdb specific vmm extensions disabled\n");
}
