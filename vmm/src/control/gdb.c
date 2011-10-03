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
#include <dr.h>
#include <insn.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;
static buffer_t    gdb_buffer;
static uint8_t     gdb_answer[GDB_ANSWER_SZ] = {GDB_ACK_BYTE, GDB_PKT_BYTE};

static inline void __gdb_reset_buffer()
{
   gdb_buffer.data.u8 = &gdb_answer[2];
   gdb_buffer.sz = 0;
}

static inline size_t __gdb_buffer_avl()
{
   return (GDB_BUF_SZ - gdb_buffer.sz);
}

static uint8_t __gdb_checksum(uint8_t *data, size_t len)
{
   uint8_t cks = 0;

   while(len--)
      cks += *data++;

   return cks;
}

static void __gdb_fix_endian(uint8_t *data, size_t len)
{
   uint16_t *fix, tmp;
   size_t   i, o, tm;

   fix = (uint16_t*)data;
   o   = 0;
   i   = len/2;
   tm  = len/4;

   while(tm--)
   {
      i--;
      tmp = fix[o];
      fix[o] = fix[i];
      fix[i] = tmp;
      o++;
   }
}

static inline void gdb_checksum(uint8_t *in, size_t len, uint8_t *out)
{
   __uint8_to_hex(out, __gdb_checksum(in, len));
}

static int gdb_checksum_verify(uint8_t *data, size_t len, uint8_t *chk)
{
   uint8_t chkv;

   if(!__hex_to_uint8(chk, &chkv))
      return 0;

   if(__gdb_checksum(data, len) != chkv)
      return 0;

   return 1;
}

void gdb_add_str(char *str, size_t len)
{
   if(gdb_buffer.sz < GDB_BUF_SZ-len)
   {
      memcpy(&gdb_buffer.data.u8[gdb_buffer.sz], str, len);
      gdb_buffer.sz += len;
   }
}

void gdb_add_byte(uint8_t value)
{
   if(gdb_buffer.sz < GDB_BUF_SZ-2)
   {
      __uint8_to_hex(&gdb_buffer.data.u8[gdb_buffer.sz], value);
      gdb_buffer.sz += 2;
   }
}

void gdb_add_number(uint64_t value, size_t precision, uint8_t endian)
{
   size_t nb = uint64_to_hex(&gdb_buffer, GDB_BUF_SZ, value, precision);

   if(endian)
      __gdb_fix_endian(&gdb_buffer.data.u8[gdb_buffer.sz - nb], nb);
}

int gdb_get_byte(uint8_t *data, size_t len, uint8_t *store)
{
   if(len < 2)
      return 0;

   return __hex_to_uint8(data, store);
}

int gdb_get_number(uint8_t *data, size_t len, uint64_t *store, uint8_t endian)
{
   if(endian)
      __gdb_fix_endian(data, len);

   return hex_to_uint64(data, len, store);
}

void gdb_send_packet()
{
   uint8_t *pkt;
   size_t  sz;

   if(!info->vmm.ctrl.dbg.status.acked)
   {
      debug(GDB_PARSE, "adding ACK byte\n");
      pkt = &gdb_answer[0];
      sz = gdb_buffer.sz + GDB_ACKPKT_SZ;
   }
   else
   {
      debug(GDB_PARSE, "already ACKed\n");
      pkt = &gdb_answer[1];
      info->vmm.ctrl.dbg.status.acked = 0;
      sz = gdb_buffer.sz + GDB_PKT_SZ;
   }

   if(gdb_buffer.sz > (GDB_ANSWER_SZ - GDB_ACKPKT_SZ))
   {
      debug(GDB, "can not send pkt with 0x%X bytes\n", gdb_buffer.sz);
      gdb_unsupported();
      return;
   }

   gdb_answer[gdb_buffer.sz + 2] = GDB_END_BYTE;
   gdb_checksum(gdb_buffer.data.u8, gdb_buffer.sz, &gdb_answer[gdb_buffer.sz+3]);
   ctrl_write(pkt, sz);

   debug(GDB_PKT, "snd (%D): [", sz);
#ifdef GDB_PKT_DBG
   debug_write(pkt, sz);
   debug_write((uint8_t*)"]\n", 2);
#endif
}

int gdb_read_mem(offset_t vaddr, uint8_t *data, size_t len)
{
   return __vm_read_mem(info->vmm.ctrl.dbg.active_cr3, vaddr, data, len);
}

int gdb_write_mem(offset_t vaddr, uint8_t *data, size_t len)
{
   return __vm_write_mem(info->vmm.ctrl.dbg.active_cr3, vaddr, data, len);
}

/*
** Query packets
*/
static void gdb_query_thread()
{
   debug(GDB_CMD, "query thread 0\n");
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

   debug(GDB_PARSE, "query unsupported\n");
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
      debug(GDB_CMD, "gdb ok for thread\n");
      gdb_ok();
      break;
   default :
      debug(GDB_CMD, "thread unsupported\n");
      gdb_unsupported();
      break;
   }
}

void gdb_send_stop_reason(uint8_t reason)
{
   size_t   rlen;
   uint32_t s_rip;

   if(__lmode64())
   {
      s_rip = 0x3a36313b;
      rlen = sizeof(uint64_t)*2;
   }
   else
   {
      /* XXX: gdb seems to wait for 32 bits regs at least */
      s_rip = 0x3a38303b;
      rlen = sizeof(uint32_t)*2;
   }

   gdb_add_str("T", 1);
   gdb_add_byte(reason);

   gdb_add_str("04:", 3);
   gdb_add_number(info->vm.cpu.gpr->rsp.raw, rlen, 1);

   gdb_add_str(";05:", 4);
   gdb_add_number(info->vm.cpu.gpr->rbp.raw, rlen, 1);

   gdb_add_str((char*)&s_rip, 4);
   gdb_add_number(__rip.raw, rlen, 1);

   gdb_add_str(";", 1);
   gdb_send_packet();
}

static void gdb_cmd_stop_reason()
{
   gdb_send_stop_reason(gdb_last_stop_reason());
}

static void gdb_cmd_rd_gpr()
{
   size_t flen, vlen, ngpr, i;

   if(__lmode64())
   {
      ngpr = 16;
      vlen = sizeof(uint64_t)*2;
   }
   else
   {
      /* XXX: gdb seems to wait for 32 bits regs at least */
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
   debug(GDB_CMD, "need to implement write all general purpose registers");
   gdb_unsupported();
}

static int __gdb_setup_mem_op(uint8_t *data, size_t len,
			      offset_t *addr, size_t *size,
			      loc_t *bytes)
{
   uint8_t  *s_addr, *s_size;
   size_t   l_addr, l_size;

   s_addr = data;

   if(!(s_size = (uint8_t*)strchr((char*)s_addr, len, ',')))
      goto __nak;

   l_addr = (size_t)(s_size - s_addr);
   s_size++;

   if(bytes)
   {
      if(!(bytes->u8 = (uint8_t*)strchr((char*)s_size, len - (l_addr+1), ':')))
	 goto __nak;

      l_size = (size_t)(bytes->u8 - s_size);
      bytes->u8++;
   }
   else
      l_size = len - (l_addr+1);

   if(!gdb_get_number(s_addr, l_addr, (uint64_t*)addr, 0))
      goto __nak;

   if(!gdb_get_number(s_size, l_size, (uint64_t*)size, 0))
      goto __nak;

   return 1;

__nak:
   gdb_nak();
   return 0;
}

static void gdb_cmd_rd_mem(uint8_t *data, size_t len)
{
   offset_t addr;
   size_t   size, need, i;
   uint8_t  store[128];

   if(!__gdb_setup_mem_op(data, len, &addr, &size, 0))
      return;

   debug(GDB_CMD, "read mem: addr 0x%X size %D\n", addr, size);

   if(size > ((GDB_ANSWER_SZ - GDB_ACKPKT_SZ)/2))
   {
      debug(GDB_CMD, "gdb buffer too small for cmd_rd_mem\n");
      gdb_unsupported();
   }

   while(size)
   {
      need = min(size, sizeof(store));

      if(!gdb_read_mem(addr, store, need))
      {
	 debug(GDB, "access failure\n");
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

   debug(GDB_CMD, "write mem: addr 0x%X size %D\n", addr, size);

   lbytes = (size_t)data + len - bytes.linear;
   if(lbytes/2 != size)
   {
      debug(GDB_CMD, "gdb cmd_wr_mem missing bytes\n");
      gdb_unsupported();
   }

   while(size)
   {
      can = min(size, sizeof(store));

      for(i=0 ; i<can ; i++, bytes.u16++)
      {
	 if(!__hex_to_uint8(bytes.u8, &store[i]))
	 {
	    debug(GDB_CMD, "gdb cmd_wr_mem invalid byte\n");
	    gdb_unsupported();
	 }
      }

      if(!gdb_write_mem(addr, store, can))
      {
	 debug(GDB, "access failure\n");
	 gdb_err_mem();
	 return;
      }

      addr += can;
      size -= can;
   }

   gdb_ok();
}

#ifdef __SVM__
static int __gdb_setup_reg(uint64_t idx, raw64_t **reg, size_t *size, uint8_t sys, uint8_t __unused__ wr)
#else
static int __gdb_setup_reg(uint64_t idx, raw64_t **reg, size_t *size, uint8_t sys, uint8_t wr)
#endif
{
   loc_t    loc;
   offset_t *cache;

   if(sys)
   {
      debug(GDB_CMD, "reg_sys_op\n");
      *size = sizeof(uint64_t);
      if(idx >= 22)
	 goto __fail;

      cache = (offset_t*)info->vm.insn_cache;
      goto __sys;
   }

   if(__lmode64())
   {
      *size = sizeof(uint64_t);
      if(idx < 16)
	 goto __gpr;
      else if(idx < 24)
	 idx -= 8;
      else
	 goto __fail;
   }
   else
   {
      /* XXX: gdb seems to wait for 32 bits regs at least */
      *size = sizeof(uint32_t);

      if(idx < 8)
	 goto __gpr;
      else if(idx >= 16)
	 goto __fail;
   }

   switch(idx)
   {
   case  8: loc.u64 = &__rip.raw;        __cond_access(wr,__rip);         goto __win;
   case  9: loc.u64 = &__rflags.raw;     __cond_access(wr,__rflags);      goto __win;

   case 10: loc.u16 = &__cs.selector.raw;__cond_access(wr,__cs.selector); goto __win16;
   case 11: loc.u16 = &__ss.selector.raw;__cond_access(wr,__ss.selector); goto __win16;
   case 12: loc.u16 = &__ds.selector.raw;__cond_access(wr,__ds.selector); goto __win16;
   case 13: loc.u16 = &__es.selector.raw;__cond_access(wr,__es.selector); goto __win16;
   case 14: loc.u16 = &__fs.selector.raw;__cond_access(wr,__fs.selector); goto __win16;
   case 15: loc.u16 = &__gs.selector.raw;__cond_access(wr,__gs.selector); goto __win16;
   }

__sys:
   switch(idx)
   {
   case  0: loc.u64 = &__cr0.raw; __cond_access(wr,__cr0); goto __win;
   case  1: loc.u64 = &__cr2.raw; __cond_access(wr,__cr2); goto __win;
   case  2: loc.u64 = &__cr3.raw; __cond_access(wr,__cr3); goto __win;
   case  3: loc.u64 = &__cr4.raw; __cond_access(wr,__cr4); goto __win;

   case  4: *cache = get_dr0(); loc.addr = (void*)cache; goto __win;
   case  5: *cache = get_dr1(); loc.addr = (void*)cache; goto __win;
   case  6: *cache = get_dr2(); loc.addr = (void*)cache; goto __win;
   case  7: *cache = get_dr3(); loc.addr = (void*)cache; goto __win;

   case  8: loc.u64 = &__dr6.raw;       __cond_access(wr,__dr6);       goto __win;
   case  9: loc.u64 = &__dr7.raw;       __cond_access(wr,__dr7);       goto __win;
   case 10: loc.u64 = &__dbgctl.raw;    __cond_access(wr,__dbgctl);    goto __win;
   case 11: loc.u64 = &__efer.raw;      /*__cond_access(wr,__efer);*/  goto __win;
   case 12: loc.u64 = &__cs.base.raw;   __cond_access(wr,__cs.base);   goto __win;
   case 13: loc.u64 = &__ss.base.raw;   __cond_access(wr,__ss.base);   goto __win;
   case 14: loc.u64 = &__ds.base.raw;   __cond_access(wr,__ds.base);   goto __win;
   case 15: loc.u64 = &__es.base.raw;   __cond_access(wr,__es.base);   goto __win;
   case 16: loc.u64 = &__fs.base.raw;   __cond_access(wr,__fs.base);   goto __win;
   case 17: loc.u64 = &__gs.base.raw;   __cond_access(wr,__gs.base);   goto __win;
   case 18: loc.u64 = &__gdtr.base.raw; __cond_access(wr,__gdtr.base); goto __win;
   case 19: loc.u64 = &__idtr.base.raw; __cond_access(wr,__idtr.base); goto __win;
   case 20: loc.u64 = &__ldtr.base.raw; __cond_access(wr,__ldtr.base); goto __win;
   case 21: loc.u64 = &__tr.base.raw;   __cond_access(wr,__tr.base);   goto __win;
   }

__gpr:
   loc.u64 = &info->vm.cpu.gpr->raw[GPR64_RAX - idx].raw;
   goto __win;

__win16:
   *size = sizeof(uint16_t);

__win:
   debug(GDB_PARSE, "reg_op win on %d\n", idx);
   *reg = (raw64_t*)loc.u64;
   return 1;

__fail:
   debug(GDB_PARSE, "reg_op failed on %d\n", idx);
   gdb_unsupported();
   return 0;
}

int __gdb_setup_reg_op(uint8_t *data, size_t len,
		       raw64_t **reg, size_t *size,
		       raw64_t *value, uint8_t wr, uint8_t sys)
{
   uint8_t  *s_idx, *s_value;
   size_t   l_idx, l_value;
   uint64_t idx;

   s_idx = data;

   if(wr)
   {
      if(!(s_value = (uint8_t*)strchr((char*)s_idx, len, '=')))
      {
	 debug(GDB_PARSE, "setup_reg_op can't parse value\n");
	 goto __nak;
      }

      l_idx = (size_t)(s_value - s_idx);
      s_value++;
      l_value = len - (l_idx+1);

      if(!gdb_get_number(s_value, l_value, (uint64_t*)value, 1))
      {
	 debug(GDB_PARSE, "setup_reg_op can't get value\n");
	 goto __nak;
      }
   }
   else
      l_idx = len;

   if(!gdb_get_number(s_idx, l_idx, &idx, 0))
   {
      debug(GDB_PARSE, "setup_reg_op can't get index\n");
      goto __nak;
   }

   return __gdb_setup_reg(idx, reg, size, sys, wr);

__nak:
   gdb_nak();
   return 0;
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

static int __gdb_setup_brk_op(uint8_t *data, size_t len,
			      uint64_t *type, size_t *kind, offset_t *addr)
{
   uint8_t  *s_type, *s_addr, *s_kind;
   size_t   l_type, l_addr, l_kind;

   s_type = data;

   if(!(s_addr = (uint8_t*)strchr((char*)s_type, len, ',')))
      goto __nak;

   l_type = (size_t)(s_addr - s_type);
   s_addr++;

   if(!(s_kind = (uint8_t*)strchr((char*)s_addr, len - (l_type+1), ',')))
      goto __nak;

   l_addr = (size_t)(s_kind - s_addr);
   s_kind++;
   l_kind = len - (l_type+1+l_addr+1);

   if(!gdb_get_number(s_type, l_type, (uint64_t*)type, 0))
      goto __nak;

   if(!gdb_get_number(s_addr, l_addr, (uint64_t*)addr, 0))
      goto __nak;

   /* XXX: gdb/remote.c:{6840,7047} kind is bp size */
   if(!gdb_get_number(s_kind, l_kind, (uint64_t*)kind, 0))
      goto __nak;

   return 1;

__nak:
   debug(GDB, "wrong setup brk op parsing\n");
   gdb_nak();
   return 0;
}

static void gdb_cmd_st_brk(uint8_t *data, size_t len)
{
   uint64_t type;
   size_t   size;
   offset_t addr;

   if(!__gdb_setup_brk_op(data, len, &type, &size, &addr))
      return;

   switch(type)
   {
   case GDB_BRK_TYPE_MEM:
      gdb_brk_mem_set(addr);
      return;
   case GDB_BRK_TYPE_HRD_X:
   case GDB_BRK_TYPE_HRD_W:
   case GDB_BRK_TYPE_HRD_RW:
      gdb_brk_hrd_set(addr, type-1, size-1);
      return;
   }

   gdb_unsupported();
}

static void gdb_cmd_rm_brk(uint8_t *data, size_t len)
{
   uint64_t type;
   size_t   size;
   offset_t addr;

   if(!__gdb_setup_brk_op(data, len, &type, &size, &addr))
      return;

   switch(type)
   {
   case GDB_BRK_TYPE_MEM:
      gdb_brk_mem_del(addr);
      return;
   case GDB_BRK_TYPE_HRD_X:
   case GDB_BRK_TYPE_HRD_W:
   case GDB_BRK_TYPE_HRD_RW:
      gdb_brk_hrd_del(addr, type-1, size-1);
      return;
   }

   gdb_unsupported();
}

static void gdb_cmd_cont(uint8_t __unused__ *data, size_t len)
{
   if(len)
      panic("need to implement continue [addr]");

   debug(GDB_CMD, "[cont] check\n");

   if(!gdb_brk_continue())
      return;

   if(gdb_brk_mem_need_restore() && !gdb_singlestep_enable(GDB_SSTEP_VMM))
      return;

   gdb_ack();
   ctrl_release_vm();
   debug(GDB_CMD, "resuming vm [cont]\n");
}

static void gdb_cmd_step(uint8_t __unused__ *data, size_t len)
{
   if(len)
      panic("need to implement single-step [addr]");

   debug(GDB_CMD, "[step] check\n");

   if(!gdb_brk_continue())
      return;

   if(!gdb_singlestep_enable(GDB_SSTEP_USER))
      return;

   gdb_ack();
   ctrl_release_vm();
   debug(GDB_CMD, "resuming vm [step]\n");
}

static void gdb_disable()
{
   gdb_set_enable(0);
   ctrl_release_vm();
}

static void gdb_enable()
{
   gdb_set_enable(1);
   gdb_set_last_stop_reason(GDB_EXIT_INT);
   ctrl_set_active();
   gdb_brk_enable();
   gdb_vmm_enable();
}

static void gdb_reset()
{
   gdb_traps_disable();
   gdb_brk_disable();
   gdb_singlestep_disable();
   gdb_vmm_disable();
   gdb_disable();
}

static void gdb_cmd_detach()
{
   debug(GDB_CMD, "gdb detach\n");
   gdb_reset();
   gdb_ok();
   gdb_wait_ack();
   return;
}

static void gdb_cmd_kill()
{
   debug(GDB_CMD, "gdb kill\n");
   gdb_reset();
   gdb_ok();
   return;
}

static void gdb_process_packet(uint8_t *data, size_t len)
{
   uint8_t cmd = *data;

   data++; len--;

   debug(GDB_PARSE, "parsing cmd %c\n", cmd);

   __gdb_reset_buffer();

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

   debug(GDB_PARSE, "cmd unsupported\n");
   gdb_unsupported();
}

static void gdb_interrupt_sequence()
{
   __gdb_reset_buffer();
   gdb_force_acked();
   gdb_preempt(GDB_EXIT_INT);
}

static size_t gdb_parse_packet(uint8_t *data, size_t len)
{
   uint8_t *ptr;
   size_t  dlen;

   if(len < GDB_PKT_SZ)
   {
      debug(GDB, "packet too small\n");
      return 0;
   }

   data++;
   ptr  = data;
   dlen = len - 1;

   while(dlen && *ptr != GDB_END_BYTE)
   {
      dlen--;
      ptr++;
   }

   if(dlen < (GDB_PKT_SZ-1))
   {
      debug(GDB, "incorrect packet: len %D, dlen %D [", len, dlen);
      data--;
      debug_write(data, len);
      debug(GDB, "]\n");

      return 0;
   }

   dlen = (size_t)(ptr - data);

   if(dlen)
   {
      ptr++;

      if(gdb_checksum_verify(data, dlen, ptr))
      {
	 debug(GDB_PARSE, "cmd packet (%D)\n", dlen);
	 gdb_process_packet(data, dlen);
      }
      else
      {
	 debug(GDB, "invalid checksum\n");
	 gdb_nak();
      }
   }

   return dlen+GDB_PKT_SZ;
}

size_t gdb_stub(uint8_t *data, size_t len)
{
   size_t done;

   debug(GDB_PKT, "rcv (%D): [", len);
#ifdef GDB_PKT_DBG
   debug_write(data, len);
   debug_write((uint8_t*)"]\n", 2);
#endif

   while(len)
   {
      switch(*data)
      {
      case GDB_ACK_BYTE:
	 if(gdb_disabled())
	 {
	    debug(GDB_CMD, "gdb connect\n");
	    gdb_enable();
	 }
	 done = 1;
	 break;

      case GDB_PKT_BYTE:
	 done = gdb_parse_packet(data, len);
	 if(!done)
	    goto __end;
	 break;

      case GDB_INT_BYTE:
	 done = 1;
	 debug(GDB_CMD, "interrupt sequence requested\n");
	 gdb_interrupt_sequence();
	 break;

      case GDB_NAK_BYTE:
	 done = 1;
	 gdb_ack();
	 break;

      default:
	 done = 1;
	 debug(GDB, "gdb_stub unsupported '\\x%x' (sz %D)\n", *data, len);
	 gdb_unsupported();
	 break;
      }

      len -= done;
      data += done;
   }

__end:
   return len;
}
