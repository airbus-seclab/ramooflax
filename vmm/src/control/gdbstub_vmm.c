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
#include <gdbstub_pkt.h>
#include <gdbstub_vmm.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static void gdb_vmm_rd_all_sysregs(uint8_t __unused__ *data, size_t __unused__ len)
{
   size_t rlen = sizeof(uint64_t)*2;

                              gdb_add_number(__cr0.raw,       rlen, 1);
   __pre_access(__cr2);       gdb_add_number(__cr2.raw,       rlen, 1);
                              gdb_add_number(__cr3.raw,       rlen, 1);
   __pre_access(__cr4);       gdb_add_number(__cr4.raw,       rlen, 1);
                              gdb_add_number(get_dr0(),       rlen, 1);
                              gdb_add_number(get_dr1(),       rlen, 1);
                              gdb_add_number(get_dr2(),       rlen, 1);
                              gdb_add_number(get_dr3(),       rlen, 1);
   __pre_access(__dr6);       gdb_add_number(__dr6.raw,       rlen, 1);
   __pre_access(__dr7);       gdb_add_number(__dr7.raw,       rlen, 1);
   __pre_access(__dbgctl);    gdb_add_number(__dbgctl.raw,    rlen, 1);
                              gdb_add_number(__efer.raw,      rlen, 1);
                              gdb_add_number(__cs.base.raw,   rlen, 1);
                              gdb_add_number(__ss.base.raw,   rlen, 1);
   __pre_access(__ds.base);   gdb_add_number(__ds.base.raw,   rlen, 1);
   __pre_access(__es.base);   gdb_add_number(__es.base.raw,   rlen, 1);
   __pre_access(__fs.base);   gdb_add_number(__fs.base.raw,   rlen, 1);
   __pre_access(__gs.base);   gdb_add_number(__gs.base.raw,   rlen, 1);
   __pre_access(__gdtr.base); gdb_add_number(__gdtr.base.raw, rlen, 1);
   __pre_access(__idtr.base); gdb_add_number(__idtr.base.raw, rlen, 1);
   __pre_access(__ldtr.base); gdb_add_number(__ldtr.base.raw, rlen, 1);
   __pre_access(__tr.base);   gdb_add_number(__tr.base.raw,   rlen, 1);

   gdb_send_packet();
}

static void gdb_vmm_wr_all_sysregs(uint8_t *data, size_t len)
{
   debug(GDBSTUB_CMD, "need to implement write all system registers\n");
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
      debug(GDBSTUB_CMD, "read sysreg failed\n");
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
      debug(GDBSTUB_CMD, "write sysreg failed\n");
      return;
   }

   if(reg == (raw64_t*)&__cr0 && __resolve_cr0_wr((cr0_reg_t*)&value) == VM_FAIL)
      goto __err;
   else if(reg == (raw64_t*)&__cr3 && __resolve_cr3_wr((cr3_reg_t*)&value) == VM_FAIL)
      goto __err;
   else if(reg == (raw64_t*)&__cr4 && __resolve_cr4_wr((cr4_reg_t*)&value) == VM_FAIL)
      goto __err;
   else
      reg->raw = value.raw;

   gdb_ok();
   return;

__err:
   debug(GDBSTUB_CMD, "invalid wr to control register\n");
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

   __setup_lbr();

   ips[0] = __lbr_from();
   ips[1] = __lbr_to();
   ips[2] = __lbr_from_excp();
   ips[3] = __lbr_to_excp();

   for(i=0 ; i<4 ; i++)
      gdb_add_number(ips[i], rlen, 0);

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

static void gdb_vmm_set_active_cr3(uint8_t *data, size_t len)
{
   cr3_reg_t val;

   if(!gdb_get_number(data, len, (uint64_t*)&val.raw, 0))
   {
      gdb_nak();
      return;
   }

   ctrl_active_cr3_enable(val);
   gdb_ok();
}

static void gdb_vmm_del_active_cr3(uint8_t __unused__ *data, size_t __unused__ len)
{
   ctrl_active_cr3_disable();
   gdb_ok();
}

static void gdb_vmm_keep_active_cr3(uint8_t __unused__ *data, size_t __unused__ len)
{
   if(ctrl_cr3_enabled())
   {
      ctrl_set_cr3_keep(1);
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

   ctrl_set_affinity(val.blow);
   gdb_ok();
   debug(GDBSTUB_CMD, "set affinity to %d\n", val.blow);
}

static int __gdb_vmm_parse_x2_arg(uint8_t *data, size_t len,
				  uint64_t *addr, uint64_t *value)
{
   if(len != sizeof(offset_t)*2*2)
      return 0;

   len /= 2;

   if(!gdb_get_number(data, len, addr, 0))
      return 0;

   data += sizeof(offset_t)*2;

   if(!gdb_get_number(data, len, value, 0))
      return 0;

   return 1;
}

static int __gdb_vmm_mem_rw_parse(uint8_t *data, size_t len,
				  loc_t *addr, size_t *sz)
{
   if(!__gdb_vmm_parse_x2_arg(data, len, &addr->raw, (uint64_t*)sz))
   {
      gdb_nak();
      return 0;
   }

   gdb_ok();
   gdb_wait_ack();
   return 1;
}

static void __gdb_vmm_rw_pmem(offset_t addr, size_t sz, uint8_t wr)
{
   vm_access_t access;

   access.addr = addr;
   access.len  = sz;
   access.wr   = wr;

   if(!__vm_remote_access_pmem(&access))
   {
      debug(GDBSTUB, "memory access failure\n");
      gdb_err_mem();
   }
}

static void gdb_vmm_rd_pmem(uint8_t *data, size_t len)
{
   loc_t  addr;
   size_t sz;

   if(!__gdb_vmm_mem_rw_parse(data, len, &addr, &sz))
      return;

   debug(GDBSTUB_CMD, "reading physical memory @ 0x%X sz %d\n", addr.linear, sz);
   __gdb_vmm_rw_pmem(addr.linear, sz, 1);
}

static void gdb_vmm_wr_pmem(uint8_t *data, size_t len)
{
   loc_t  addr;
   size_t sz;

   if(!__gdb_vmm_mem_rw_parse(data, len, &addr, &sz))
      return;

   debug(GDBSTUB_CMD, "writing physical memory @ 0x%X sz %d\n", addr.linear, sz);
   __gdb_vmm_rw_pmem(addr.linear, sz, 0);
}

static void gdb_vmm_rd_vmem(uint8_t *data, size_t len)
{
   loc_t  addr;
   size_t sz;

   if(!__gdb_vmm_mem_rw_parse(data, len, &addr, &sz))
      return;

   debug(GDBSTUB_CMD, "reading virtual memory @ 0x%X sz %d\n", addr.linear, sz);
   if(!gdb_mem_send(addr.linear, sz))
   {
      debug(GDBSTUB, "memory access failure\n");
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

   debug(GDBSTUB_CMD, "writing virtual memory @ 0x%X sz %d\n", addr.linear, sz);
   if(!gdb_mem_recv(addr.linear, sz))
   {
      debug(GDBSTUB, "memory access failure\n");
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

   debug(GDBSTUB_CMD, "translating 0x%X\n", vaddr);

   if(!__paging())
      paddr = vaddr;
   else if(!__pg_walk(info->vmm.ctrl.active_cr3, vaddr, &paddr, &psz, 1))
   {
      debug(GDBSTUB, "memory translation failure\n");
      gdb_err_mem();
      return;
   }

   debug(GDBSTUB_CMD, "sending 0x%X\n", paddr);

   if(__lmode64())
      sz = sizeof(uint64_t)*2;
   else
      /* XXX: gdb seems to wait for 32 bits regs at least */
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

   ctrl_usr_set_cr_rd(val.wlow);
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

   debug(GDBSTUB_CMD, "write cr mask 0x%x\n", val.wlow);

   ctrl_usr_set_cr_wr(val.wlow);
   gdb_ok();
}

static void gdb_vmm_clear_idt_event(uint8_t __unused__ *data, size_t __unused__ len)
{
   if(__injecting_event())
      __clear_event_injection();

   gdb_ok();
}

static void gdb_vmm_get_idt_event(uint8_t __unused__ *data, size_t __unused__ len)
{
   gdb_add_number(__injected_event(), sizeof(uint64_t)*2, 0);
   gdb_send_packet();
}

static void gdb_vmm_rdtsc(uint8_t __unused__ *data, size_t __unused__ len)
{
   gdb_add_number(rdtsc(), sizeof(uint64_t)*2, 0);
   gdb_send_packet();
}

static void gdb_vmm_can_cli(uint8_t __unused__ *data, size_t __unused__ len)
{
   bool_t can;

   if(__interrupt_shadow || (__injecting_event() && __injected_event_type == 0))
      can = 0;
   else
      can = 1;

   gdb_add_number(can, sizeof(uint8_t)*2, 0);
   gdb_send_packet();
}

/*
** Try to get a nested pte for the given addr
** Directly try to remap VM memory with finest
** granularity to return a nested pte
*/
static void gdb_vmm_npg_get_pte(uint8_t *data, size_t len)
{
   loc_t        addr;
   npg_pte64_t  *pte;

   if(!gdb_get_number(data, len, (uint64_t*)&addr.raw, 0))
   {
      gdb_nak();
      return;
   }

   pte = _npg_remap_finest_4K(addr.linear);
   if(!pte)
   {
      debug(GDBSTUB_CMD, "no npg pte for 0x%x\n", addr.raw);
      gdb_err_mem();
      return;
   }

   npg_invlpg(addr.linear);
   gdb_add_number(pte->raw, sizeof(uint64_t)*2, 0);
   gdb_send_packet();
}

static void gdb_vmm_npg_set_pte(uint8_t *data, size_t len)
{
   loc_t        addr;
   npg_pte64_t  npte, *opte;

   if(!__gdb_vmm_parse_x2_arg(data, len, &addr.raw, &npte.raw))
   {
      gdb_nak();
      return;
   }

   opte = _npg_get_pte(addr.linear);
   if(!opte)
   {
      debug(GDBSTUB_CMD, "no npg pte for 0x%x\n", addr.raw);
      gdb_err_mem();
      return;
   }

   opte->raw = npte.raw;
   npg_invlpg(addr.linear);
   gdb_ok();
}

static void gdb_vmm_get_fault(uint8_t __unused__ *data, size_t __unused__ len)
{
   fault_ctx_t *fault = &info->vm.cpu.fault;

   gdb_add_number(fault->excp.err,     8, 0);
   gdb_add_number(fault->npf.err.raw, 16, 0);
   gdb_add_number(fault->npf.vaddr,   16, 0);
   gdb_add_number(fault->npf.paddr,   16, 0);
   gdb_send_packet();
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
   gdb_vmm_clear_idt_event,
   gdb_vmm_rdtsc,
   gdb_vmm_npg_get_pte,
   gdb_vmm_npg_set_pte,
   gdb_vmm_get_fault,
   gdb_vmm_get_idt_event,
   gdb_vmm_can_cli,
};

void gdb_cmd_vmm(uint8_t *data, size_t len)
{
   uint8_t idx = *data - 0x80;

   data++; len--;

   if(idx <= sizeof(gdb_vmm_handlers)/sizeof(gdb_vmm_hdl_t))
   {
      debug(GDBSTUB_CMD, "gdb_vmm handler call %d\n", idx);
      gdb_vmm_handlers[idx](data, len);
      return;
   }

   debug(GDBSTUB_CMD, "vmm cmd unsupported\n");
   gdb_unsupported();
}

void gdb_vmm_enable()
{
}

void gdb_vmm_disable()
{
   ctrl_active_cr3_reset();
   ctrl_usr_reset();
}
