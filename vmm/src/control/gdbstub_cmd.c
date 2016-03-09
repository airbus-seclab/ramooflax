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
#include <gdbstub.h>
#include <gdbstub_pkt.h>
#include <gdbstub_cmd.h>
#include <gdbstub_vmm.h>
#include <dbg.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

/*
** Query packets
*/
static void gdb_query_thread()
{
   debug(GDBSTUB_CMD, "query thread 0\n");
   gdb_add_str("QC0", 3);
   gdb_send_packet();
}

/*
** Command packets
*/
static void gdb_cmd_query(uint8_t *data, size_t len)
{
   uint8_t qry = *data;

   data++; len--;

   switch(qry)
   {
   case GDB_QUERY_THREAD: gdb_query_thread(); return;
   }

   debug(GDBSTUB_CMD, "query unsupported\n");
   gdb_unsupported();
   return;
}

static void gdb_cmd_thread(uint8_t *data)
{
   switch(*data)
   {
   case 'g':
   case 'c':
      /* ignore thread-id */
      debug(GDBSTUB_CMD, "gdb ok for thread\n");
      gdb_ok();
      break;
   default :
      debug(GDBSTUB_CMD, "thread unsupported\n");
      gdb_unsupported();
      break;
   }
}

static void gdb_cmd_stop_reason()
{
   gdb_send_stop_reason(gdb_last_stop_reason());
}

static void gdb_cmd_rd_gpr()
{
   size_t flen, vlen, ngpr, i;

   if(cpu_addr_sz() == 64)
   {
      ngpr = 16;
      vlen = sizeof(uint64_t)*2;
   }
   else /* XXX: gdb seems to wait for 32 bits regs at least */
   {
      ngpr = 8;
      vlen = sizeof(uint32_t)*2;
   }

   flen = sizeof(uint32_t)*2;

   /* [r/e]ax - [r/e]di */
   for(i=GPR64_RAX ; i >= ((GPR64_RAX+1)-ngpr) ; i--)
      gdb_add_number(info->vm.cpu.gpr->raw[i].raw, vlen, 1);

   /* [r/e]ip */
   gdb_add_number(__rip.raw, vlen, 1);

   /* fixed length eflags, cs, ss, ds, es, fs, gs */
                                gdb_add_number(__rflags.raw,      flen, 1);
                                gdb_add_number(__cs.selector.raw, flen, 1);
   __pre_access(__ss.selector); gdb_add_number(__ss.selector.raw, flen, 1);
   __pre_access(__ds.selector); gdb_add_number(__ds.selector.raw, flen, 1);
   __pre_access(__es.selector); gdb_add_number(__es.selector.raw, flen, 1);
   __pre_access(__fs.selector); gdb_add_number(__fs.selector.raw, flen, 1);
   __pre_access(__gs.selector); gdb_add_number(__gs.selector.raw, flen, 1);

   gdb_send_packet();
}

static void gdb_cmd_wr_gpr(uint8_t __unused__ *data, size_t __unused__ len)
{
   debug(GDBSTUB_CMD, "need to implement write all general purpose registers");
   gdb_unsupported();
}


static void gdb_cmd_rd_mem(uint8_t *data, size_t len)
{
   offset_t addr;
   size_t   size, need, i;
   uint8_t  store[128];

   if(!__gdb_setup_mem_op(data, len, &addr, &size, 0))
      return;

   debug(GDBSTUB_CMD, "read mem: addr 0x%X size %D\n", addr, size);

   if(size > ((GDB_ANSWER_SZ - GDB_ACKPKT_SZ)/2))
   {
      debug(GDBSTUB_CMD, "gdb buffer too small for cmd_rd_mem\n");
      gdb_unsupported();
   }

   while(size)
   {
      need = min(size, sizeof(store));

      if(gdb_vmem_read(addr, store, need) != VM_DONE)
      {
	 debug(GDBSTUB_CMD, "access failure\n");
	 gdb_err_mem();
	 return;
      }

      for(i=0 ; i<need ; i++)
	 gdb_add_byte(store[i]);

      addr += need;
      size -= need;
   }

   gdb_send_packet();
}

static void gdb_cmd_wr_mem(uint8_t *data, size_t len)
{
   offset_t addr;
   loc_t    bytes;
   size_t   size, lbytes, can, i;
   uint8_t  store[128];

   if(!__gdb_setup_mem_op(data, len, &addr, &size, &bytes))
      return;

   debug(GDBSTUB_CMD, "write mem: addr 0x%X size %D\n", addr, size);

   lbytes = (size_t)data + len - bytes.linear;
   if(lbytes/2 != size)
   {
      debug(GDBSTUB_CMD, "gdb cmd_wr_mem missing bytes\n");
      gdb_unsupported();
   }

   while(size)
   {
      can = min(size, sizeof(store));

      for(i=0 ; i<can ; i++, bytes.u16++)
      {
	 if(!__hex_to_uint8(bytes.u8, &store[i]))
	 {
	    debug(GDBSTUB_CMD, "gdb cmd_wr_mem invalid byte\n");
	    gdb_unsupported();
	 }
      }

      if(gdb_vmem_write(addr, store, can) != VM_DONE)
      {
	 debug(GDBSTUB_CMD, "access failure\n");
	 gdb_err_mem();
	 return;
      }

      addr += can;
      size -= can;
   }

   gdb_ok();
}

static void gdb_cmd_rd_reg(uint8_t *data, size_t len)
{
   raw64_t *reg;
   size_t  size;

   if(!__gdb_setup_reg_op(data, len, &reg, &size, 0, 0, 0))
      return;

   size *= 2; /* to nibbles */
   gdb_add_number(reg->raw, size, 1);
   gdb_send_packet();
}

static void gdb_cmd_wr_reg(uint8_t *data, size_t len)
{
   raw64_t  *reg, value;
   size_t   size;

   if(!__gdb_setup_reg_op(data, len, &reg, &size, &value, 1, 0))
      return;

   switch(size)
   {
   case 2: reg->wlow = value.wlow; break;
   case 4: reg->low  = value.low;  break;
   case 8: reg->raw  = value.raw;  break;
   }

   gdb_ok();
}
static void gdb_cmd_st_brk(uint8_t *data, size_t len)
{
   uint64_t type;
   size_t   size;
   offset_t addr;
   int      rc;

   if(!__gdb_setup_brk_op(data, len, &type, &size, &addr))
      return;

   switch(type)
   {
   case GDB_BRK_TYPE_MEM:
      rc = dbg_soft_set(addr, 0);
      break;
   case GDB_BRK_TYPE_HRD_X:
   case GDB_BRK_TYPE_HRD_W:
   case GDB_BRK_TYPE_HRD_RW:
      rc = dbg_hard_brk_set(addr, type-1, size-1, 0);
      break;
   default:
      gdb_unsupported();
      return;
   }

   if(rc == VM_DONE)
   {
      gdb_ok();
      return;
   }

   gdb_err_mem();
}

static void gdb_cmd_rm_brk(uint8_t *data, size_t len)
{
   uint64_t type;
   size_t   size;
   offset_t addr;
   int      rc;

   if(!__gdb_setup_brk_op(data, len, &type, &size, &addr))
      return;

   switch(type)
   {
   case GDB_BRK_TYPE_MEM:
      rc = dbg_soft_del(addr);
      break;
   case GDB_BRK_TYPE_HRD_X:
   case GDB_BRK_TYPE_HRD_W:
   case GDB_BRK_TYPE_HRD_RW:
      rc = dbg_hard_brk_del(addr, type-1, size-1);
      break;
   default:
      gdb_unsupported();
      return;
   }

   if(rc == VM_DONE)
   {
      gdb_ok();
      return;
   }

   gdb_err_mem();
}

static void __gdb_cmd_resume(uint8_t stp)
{
   if(dbg_resume(stp) == VM_DONE)
   {
      gdb_ack();
      gdb_set_lock(0);
      return;
   }

   gdb_err();
}

static void gdb_cmd_cont(uint8_t __unused__ *data, size_t len)
{
   if(len)
      panic("need to implement continue [addr]");

   debug(GDBSTUB_CMD, "[cont] check\n");
   __gdb_cmd_resume(0);
   debug(GDBSTUB_CMD, "resuming vm [cont]\n");
}

static void gdb_cmd_step(uint8_t __unused__ *data, size_t len)
{
   if(len)
      panic("need to implement single-step [addr]");

   debug(GDBSTUB_CMD, "[step] check\n");
   __gdb_cmd_resume(1);
   debug(GDBSTUB_CMD, "resuming vm [step]\n");
}

static void gdb_cmd_detach()
{
   debug(GDBSTUB_CMD, "gdb detach\n");
   gdb_reset();
   gdb_ok();
   gdb_wait_ack();
   return;
}

static void gdb_cmd_kill()
{
   debug(GDBSTUB_CMD, "gdb kill\n");
   gdb_reset();
   gdb_ok();
   return;
}

void gdb_process_packet(uint8_t *data, size_t len)
{
   uint8_t cmd = *data;

   data++; len--;

   debug(GDBSTUB_CMD, "parsing cmd %c\n", cmd);

   switch(cmd)
   {
   case GDB_CMD_R_MEM:
      return gdb_cmd_rd_mem(data, len);
   case GDB_CMD_W_MEM:
      return gdb_cmd_wr_mem(data, len);
   case GDB_CMD_R_REG:
      return gdb_cmd_rd_reg(data, len);
   case GDB_CMD_W_REG:
      return gdb_cmd_wr_reg(data, len);
   case GDB_CMD_S_BRK:
      return gdb_cmd_st_brk(data, len);
   case GDB_CMD_R_BRK:
      return gdb_cmd_rm_brk(data, len);
   case GDB_CMD_R_GPR:
      return gdb_cmd_rd_gpr();
   case GDB_CMD_W_GPR:
      return gdb_cmd_wr_gpr(data, len);
   case GDB_CMD_CONT:
      return gdb_cmd_cont(data, len);
   case GDB_CMD_STEP:
      return gdb_cmd_step(data, len);
   case GDB_CMD_THREAD:
      return gdb_cmd_thread(data);
   case GDB_CMD_STOP_REASON:
      return gdb_cmd_stop_reason();
   case GDB_CMD_VMM:
      return gdb_cmd_vmm(data, len);
   case GDB_CMD_QUERY:
      return gdb_cmd_query(data, len);
   case GDB_CMD_KILL:
      return gdb_cmd_kill();
   case GDB_CMD_DETACH:
      return gdb_cmd_detach();
   }

   debug(GDBSTUB_CMD, "cmd unsupported\n");
   gdb_unsupported();
}
