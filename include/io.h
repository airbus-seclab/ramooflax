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
#ifndef __IO_H__
#define __IO_H__

#include <types.h>

#ifndef __INIT__
#ifdef CONFIG_ARCH_AMD
#include <svm_exit_io.h>
#define __io_init(_io_)           __svm_io_init(_io_)
#else
#include <vmx_exit_io.h>
#define __io_init(_io_)           __vmx_io_init(_io_)
#endif
#endif

/*
** I/O instruction format
*/
typedef struct io_insn
{
   union
   {
      struct
      {
	 uint16_t in:1;   /* in or out */
	 uint16_t s:1;    /* string operation */
	 uint16_t sz:3;   /* operand size 1/2/4 (1,2,4) */
	 uint16_t addr:3; /* addr size 16/32/64 (1,2,4) */
	                  /* only for ins/outs SI/ESI/RSI, CX,ECX,RCX */
	 uint16_t back:1; /* backward string op */
	 uint16_t rep:1;  /* rep prefix */

      } __attribute__((packed));

      uint16_t raw;

   } __attribute__((packed));

   uint16_t port;
   size_t   msk;
   loc_t    src;
   loc_t    dst;

} __attribute__((packed)) io_insn_t;

/*
** I/O device size information
**
** available : number of bytes available in device memory
** done      : number of bytes transfered to/from device memory
** miss      : number of bytes not transfered to/from device memory
*/
typedef struct io_size
{
   size_t available;
   size_t done;
   size_t miss;

} io_size_t;

typedef int (*io_flt_hdl_t)(void*);

/*
** Functions
*/
int  dev_io_insn(io_insn_t*, void*, io_size_t*);
int  dev_io_native(io_insn_t*, void*);
int  dev_io_proxify(io_insn_t*);
int  dev_io_proxify_filter(io_insn_t*, io_flt_hdl_t);

#endif
