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
#include <dr.h>
#include <insn.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;

static buffer_t    gdb_buffer;
static uint8_t     gdb_input[GDB_INPUT_SZ];
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

static inline uint8_t* gdb_escape(uint8_t *data, uint8_t byte)
{
   *data++ = GDB_ESC_BYTE;
   *data++ = byte ^ GDB_ESC_XOR;

   return data;
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

static size_t gdb_parse_packet(uint8_t *data, size_t len)
{
   uint8_t *ptr;
   size_t  dlen;

   if(len < GDB_PKT_SZ)
   {
      debug(GDBSTUB_PKT, "packet too small\n");
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
      debug(GDBSTUB_PKT, "incorrect packet: len %D, dlen %D [", len, dlen);
#ifdef CONFIG_GDBSTUB_PKT_DBG
      data--;
      debug_write(data, len);
#endif
      debug(GDBSTUB_PKT, "]\n");
      return 0;
   }

   dlen = (size_t)(ptr - data);

   if(dlen)
   {
      ptr++;

      if(gdb_checksum_verify(data, dlen, ptr))
      {
	 debug(GDBSTUB_PKT, "cmd packet (%D)\n", dlen);
	 __gdb_reset_buffer();
	 gdb_process_packet(data, dlen);
      }
      else
      {
	 debug(GDBSTUB_PKT, "invalid checksum\n");
	 gdb_nak();
      }
   }

   return dlen+GDB_PKT_SZ;
}

static void gdb_interrupt_sequence()
{
   __gdb_reset_buffer();
   gdb_set_ack(1);
   __gdb_preempt(GDB_EXIT_INT);
}

static size_t gdb_consume_packet(uint8_t *data, size_t len)
{
   size_t done;

   debug(GDBSTUB_PKT, "consume (%D): [", len);
#ifdef CONFIG_GDBSTUB_PKT_DBG
   debug_write(data, len);
   debug_write((uint8_t*)"]\n", 2);
#endif

   while(len)
   {
      switch(*data)
      {
      case GDB_ACK_BYTE:
	 if(!gdb_enabled())
	 {
	    debug(GDBSTUB_PKT, "gdb connect\n");
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
	 debug(GDBSTUB_PKT, "interrupt sequence requested\n");
	 gdb_interrupt_sequence();
	 break;

      case GDB_NAK_BYTE:
	 done = 1;
	 gdb_ack();
	 break;

      default:
	 done = 1;
	 debug(GDBSTUB_PKT, "gdb_stub unsupported '\\x%x' (sz %D)\n", *data, len);
	 gdb_unsupported();
	 break;
      }

      len -= done;
      data += done;
   }

__end:
   return len;
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

int __gdb_setup_reg(uint64_t idx, raw64_t **reg,
		    size_t *size, uint8_t sys,
		    uint8_t
#ifdef CONFIG_ARCH_AMD
		    __unused__
#endif
		    wr)
{
   loc_t    loc;
   offset_t *cache;

   if(sys)
   {
      debug(GDBSTUB_PKT, "reg_sys_op\n");
      *size = sizeof(uint64_t);
      if(idx >= 22)
	 goto __fail;

      cache = (offset_t*)info->vm.cpu.insn_cache;
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
   debug(GDBSTUB_PKT, "reg_op win on %d\n", idx);
   *reg = (raw64_t*)loc.u64;
   return 1;

__fail:
   debug(GDBSTUB_PKT, "reg_op failed on %d\n", idx);
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
	 debug(GDBSTUB_PKT, "setup_reg_op can't parse value\n");
	 goto __nak;
      }

      l_idx = (size_t)(s_value - s_idx);
      s_value++;
      l_value = len - (l_idx+1);

      if(!gdb_get_number(s_value, l_value, (uint64_t*)value, 1))
      {
	 debug(GDBSTUB_PKT, "setup_reg_op can't get value\n");
	 goto __nak;
      }
   }
   else
      l_idx = len;

   if(!gdb_get_number(s_idx, l_idx, &idx, 0))
   {
      debug(GDBSTUB_PKT, "setup_reg_op can't get index\n");
      goto __nak;
   }

   return __gdb_setup_reg(idx, reg, size, sys, wr);

__nak:
   gdb_nak();
   return 0;
}

int __gdb_setup_mem_op(uint8_t *data, size_t len,
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

int __gdb_setup_brk_op(uint8_t *data, size_t len,
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
   debug(GDBSTUB_PKT, "wrong setup brk op parsing\n");
   gdb_nak();
   return 0;
}

void gdb_ack()
{
   uint8_t x = GDB_ACK_BYTE;
   gdb_io_write(&x, 1);
   gdb_set_ack(1);
}

void gdb_nak()
{
   uint8_t x = GDB_NAK_BYTE;
   gdb_io_write(&x, 1);
}

void gdb_wait_ack()
{
   uint8_t x;
   while(gdb_io_read(&x, 1) != 1 && x != GDB_ACK_BYTE);
}

void gdb_send_stop_reason(uint8_t reason)
{
   size_t   rlen;
   uint32_t s_rip;
   uint64_t mode;

   if(__lmode64())
   {
      mode = 64;
      s_rip = 0x3a36313b;
      rlen = sizeof(uint64_t)*2;
   }
   else
   {
      if(__pmode32())
	 mode = 32;
      else
	 mode = 16;

      /* XXX: gdb seems to wait for 32 bits regs at least */
      s_rip = 0x3a38303b;
      rlen = sizeof(uint32_t)*2;
   }

   gdb_add_str("T", 1);
   gdb_add_byte(reason);

   gdb_add_str("md:", 3);
   gdb_add_number(mode, 2, 0);

   gdb_add_str(";04:", 4);
   gdb_add_number(info->vm.cpu.gpr->rsp.raw, rlen, 1);

   gdb_add_str(";05:", 4);
   gdb_add_number(info->vm.cpu.gpr->rbp.raw, rlen, 1);

   gdb_add_str((char*)&s_rip, 4);
   gdb_add_number(__rip.raw, rlen, 1);

   gdb_add_str(";", 1);
   gdb_send_packet();
}

void gdb_recv_packet()
{
   uint8_t *ptr;
   size_t  len, remain = 0;

   do
   {
      do
      {
	 len = remain;
	 remain = 0;
	 ptr = &gdb_input[len];
	 len += gdb_io_read(ptr, sizeof(gdb_input) - len);

	 if(len)
	 {
	    remain = gdb_consume_packet(gdb_input, len);
	    debug(GDBSTUB_PKT, "remain (%D)\n", remain);

	    if(remain && remain != len)
	    {
	       ptr = gdb_input + (len - remain);
	       memcpy(gdb_input, ptr, remain);
	    }
	 }
      } while(remain);
   } while(gdb_locked());
}

void gdb_send_packet()
{
   uint8_t *pkt;
   size_t  sz;

   if(!gdb_acked())
   {
      debug(GDBSTUB_PKT, "adding ACK byte\n");
      pkt = &gdb_answer[0];
      sz = gdb_buffer.sz + GDB_ACKPKT_SZ;
   }
   else
   {
      debug(GDBSTUB_PKT, "already ACKed\n");
      pkt = &gdb_answer[1];
      gdb_set_ack(0);
      sz = gdb_buffer.sz + GDB_PKT_SZ;
   }

   if(gdb_buffer.sz > (GDB_ANSWER_SZ - GDB_ACKPKT_SZ))
   {
      debug(GDBSTUB_PKT, "can not send pkt with 0x%X bytes\n", gdb_buffer.sz);
      gdb_unsupported();
      return;
   }

   gdb_answer[gdb_buffer.sz + 2] = GDB_END_BYTE;
   gdb_checksum(gdb_buffer.data.u8, gdb_buffer.sz, &gdb_answer[gdb_buffer.sz+3]);
   gdb_io_write(pkt, sz);

   debug(GDBSTUB_PKT, "gdb sent (%D): [", sz);
#ifdef CONFIG_GDBSTUB_PKT_DBG
   debug_write(pkt, sz);
   debug_write((uint8_t*)"]\n", 2);
#endif
}
