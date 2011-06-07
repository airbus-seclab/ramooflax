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
#ifdef __SVM__
#include <svm_exit_io.h>
#define __io_init(_io_)           __svm_io_init(_io_)
//#define ___io_setup(_io_)         __svm_io_setup(_io_)
#else
#include <vmx_exit_io.h>
#define __io_init(_io_)           __vmx_io_init(_io_)
//#define ___io_setup(_io_)         __vmx_io_setup(_io_)
#endif
#endif

/*
** I/O instruction format
*/
typedef struct io_insn
{
   struct
   {
      uint8_t sz:3;     /* i/o for 1, 2 or 4 bytes */
      uint8_t rep:1;    /* rep prefix */
      uint8_t s:1;      /* string insn */
      uint8_t d:1;      /* direction (0) out, (1) in */
      uint8_t bck:1;    /* backward */
      uint8_t pg:1;     /* paging enabled */

   } __attribute__((packed));

   uint8_t    seg;      /* segment override prefix (ES only for now) */
   uint32_t   mask;     /* 0xffff or 0xffffffff */
   uint16_t   port;     /* i/o port */
   uint32_t   ds, es;   /* segment base addr */

   loc_t      src;
   loc_t      dst;

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
   uint32_t  available;
   uint32_t  done;
   uint32_t  miss;

} io_size_t;

/*
** I/O base operations macros
*/
#define  __io_insn_byte_op(_dSt_,_sRc_,_oPd_,_oPs_)  \
   ( *(_dSt_).u8  _oPd_ = *(_sRc_).u8  _oPs_ )
#define  __io_insn_word_op(_dSt_,_sRc_,_oPd_,_oPs_)  \
   ( *(_dSt_).u16 _oPd_ = *(_sRc_).u16 _oPs_ )
#define  __io_insn_long_op(_dSt_,_sRc_,_oPd_,_oPs_)  \
   ( *(_dSt_).u32 _oPd_ = *(_sRc_).u32 _oPs_ )

#define  __io_insn_byte(_dSt_,_sRc_)            __io_insn_byte_op(_dSt_,_sRc_,,)
#define  __io_insn_word(_dSt_,_sRc_)            __io_insn_word_op(_dSt_,_sRc_,,)
#define  __io_insn_long(_dSt_,_sRc_)            __io_insn_long_op(_dSt_,_sRc_,,)

/*
** OUTS/INS forward
*/
#define  __io_insn_byte_inc(_dSt_,_sRc_)        __io_insn_byte_op(_dSt_,_sRc_,++,++)
#define  __io_insn_word_inc(_dSt_,_sRc_)        __io_insn_word_op(_dSt_,_sRc_,++,++)
#define  __io_insn_long_inc(_dSt_,_sRc_)        __io_insn_long_op(_dSt_,_sRc_,++,++)

/*
** INS backward
*/
#define  __io_insn_byte_dec_inc(_dSt_,_sRc_)    __io_insn_byte_op(_dSt_,_sRc_,--,++)
#define  __io_insn_word_dec_inc(_dSt_,_sRc_)    __io_insn_word_op(_dSt_,_sRc_,--,++)
#define  __io_insn_long_dec_inc(_dSt_,_sRc_)    __io_insn_long_op(_dSt_,_sRc_,--,++)

/*
** OUTS backward
*/
#define  __io_insn_byte_inc_dec(_dSt_,_sRc_)    __io_insn_byte_op(_dSt_,_sRc_,++,--)
#define  __io_insn_word_inc_dec(_dSt_,_sRc_)    __io_insn_word_op(_dSt_,_sRc_,++,--)
#define  __io_insn_long_inc_dec(_dSt_,_sRc_)    __io_insn_long_op(_dSt_,_sRc_,++,--)

#define __io_insn_string_loop(_dSt_,_sRc_,_sz_,_unit_,_operator_)	\
   ({									\
      while( (_sz_)->miss && (_sz_)->available >= (_unit_) )		\
      {									\
	 _operator_( (_dSt_), (_sRc_) );				\
	 								\
	 (_sz_)->done      += (_unit_);					\
	 (_sz_)->available -= (_unit_);					\
	 (_sz_)->miss      -= (_unit_);					\
      }									\
   })

#define __io_insn_string_in_fwd_byte(_dSt_,_sRc_,_sz_)		\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,1,__io_insn_byte_inc)
#define __io_insn_string_in_fwd_word(_dSt_,_sRc_,_sz_)		\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,2,__io_insn_word_inc)
#define __io_insn_string_in_fwd_long(_dSt_,_sRc_,_sz_)		\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,4,__io_insn_long_inc)

#define __io_insn_string_out_fwd_byte(_dSt_,_sRc_,_sz_)		\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,1,__io_insn_byte_inc)
#define __io_insn_string_out_fwd_word(_dSt_,_sRc_,_sz_)		\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,2,__io_insn_word_inc)
#define __io_insn_string_out_fwd_long(_dSt_,_sRc_,_sz_)		\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,4,__io_insn_long_inc)

#define __io_insn_string_in_bwd_byte(_dSt_,_sRc_,_sz_)			\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,1,__io_insn_byte_dec_inc)
#define __io_insn_string_in_bwd_word(_dSt_,_sRc_,_sz_)			\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,2,__io_insn_word_dec_inc)
#define __io_insn_string_in_bwd_long(_dSt_,_sRc_,_sz_)			\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,4,__io_insn_long_dec_inc)

#define __io_insn_string_out_bwd_byte(_dSt_,_sRc_,_sz_)			\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,1,__io_insn_byte_inc_dec)
#define __io_insn_string_out_bwd_word(_dSt_,_sRc_,_sz_)			\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,2,__io_insn_word_inc_dec)
#define __io_insn_string_out_bwd_long(_dSt_,_sRc_,_sz_)			\
   __io_insn_string_loop(_dSt_,_sRc_,_sz_,4,__io_insn_long_inc_dec)

#define __io_insn_string_gen(_dir1_,_dir2_,_dSt_,_sRc_,_sz_,_unit_)	\
   ({									\
      switch( (_unit_) )						\
      {									\
      case 1:								\
	 __io_insn_string_##_dir1_##_##_dir2_##_byte(_dSt_,_sRc_,_sz_);	\
	 break;								\
      case 2:								\
	 __io_insn_string_##_dir1_##_##_dir2_##_word(_dSt_,_sRc_,_sz_);	\
	 break;								\
      case 4:								\
	 __io_insn_string_##_dir1_##_##_dir2_##_long(_dSt_,_sRc_,_sz_);	\
	 break;								\
      }									\
   })

#define __io_insn_string_in_fwd(_dSt_,_sRc_,_sz_,_unit_)	\
   __io_insn_string_gen(in,fwd,_dSt_,_sRc_,_sz_,_unit_)
#define __io_insn_string_in_bwd(_dSt_,_sRc_,_sz_,_unit_)	\
   __io_insn_string_gen(in,bwd,_dSt_,_sRc_,_sz_,_unit_)
#define __io_insn_string_out_fwd(_dSt_,_sRc_,_sz_,_unit_)	\
   __io_insn_string_gen(out,fwd,_dSt_,_sRc_,_sz_,_unit_)
#define __io_insn_string_out_bwd(_dSt_,_sRc_,_sz_,_unit_)	\
   __io_insn_string_gen(out,bwd,_dSt_,_sRc_,_sz_,_unit_)


/*
** Functions
*/
int     dev_io_insn(io_insn_t*, void*, io_size_t*);

#endif

