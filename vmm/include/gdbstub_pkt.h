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
#ifndef __GDB_STUB_PKT_H__
#define __GDB_STUB_PKT_H__

#include <types.h>
#include <gdbstub.h>

/*
** Packet structure
*/
#define GDB_PKT_SZ             4
#define GDB_ACKPKT_SZ          (GDB_PKT_SZ+1)

#define GDB_INPUT_SZ           1024
#define GDB_ANSWER_SZ          1024
#define GDB_BUF_SZ             (GDB_ANSWER_SZ - GDB_ACKPKT_SZ)

#define GDB_PKT_BYTE           '$'
#define GDB_END_BYTE           '#'
#define GDB_ACK_BYTE           '+'
#define GDB_NAK_BYTE           '-'
#define GDB_NTF_BYTE           '%'
#define GDB_RLN_BYTE           '*'
#define GDB_ESC_BYTE           '}'
#define GDB_ESC_XOR            ' '
/* CTL+C */
#define GDB_INT_BYTE            3

/*
** Functions
*/
#define gdb_ok()                       gdb_io_write((uint8_t*)"+$OK#9a",  7)
#define gdb_unsupported()              gdb_io_write((uint8_t*)"+$#00",    5)

#define gdb_err()                      gdb_err_gen()
#define gdb_err_gen()                  gdb_io_write((uint8_t*)"+$E00#a5", 8)
#define gdb_err_mem()                  gdb_io_write((uint8_t*)"+$E0e#da", 8)
#define gdb_err_oom()                  gdb_io_write((uint8_t*)"+$E0c#d8", 8)
#define gdb_err_inv()                  gdb_io_write((uint8_t*)"+$E16#ac", 8)

void gdb_ack();
void gdb_nak();
void gdb_wait_ack();

void gdb_add_str(char*, size_t);
void gdb_add_byte(uint8_t);
void gdb_add_number(uint64_t, size_t, uint8_t);
int  gdb_get_byte(uint8_t*, size_t, uint8_t*);
int  gdb_get_number(uint8_t*, size_t, uint64_t*, uint8_t);

int __gdb_setup_reg_op(uint8_t*, size_t,
                       raw64_t**, size_t*,
                       raw64_t*, uint8_t, uint8_t);
int  __gdb_setup_brk_op(uint8_t*, size_t, uint64_t*, size_t*, offset_t*);
int  __gdb_setup_mem_op(uint8_t*, size_t, offset_t*, size_t*, loc_t*);

void gdb_send_packet();
void gdb_recv_packet();
void gdb_send_stop_reason(uint8_t);

#endif
