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
#ifndef __TYPES_H__
#define __TYPES_H__

#define __aligned(addr,val)    (!((addr)&((val)-1)))
#define __align(addr,val)      ((addr)&(~((val)-1)))
#define __align_next(addr,val) __align(addr+val,val)

#define int_aligned(addr)      __aligned(addr,sizeof(int))
#define int_align(addr)        __align(addr,sizeof(int))
#define int_align_next(addr)   __align_next(addr,sizeof(int))

#define long_aligned(addr)     __aligned(addr,sizeof(long))
#define long_align(addr)       __align(addr,sizeof(long))
#define long_align_next(addr)  __align_next(addr,sizeof(long))

#define __range_inc(_v,_s,_e) (((_v) >= (_s)) && ((_v) <= (_e)))
#define __range_exc(_v,_s,_e) (((_v) >= (_s)) && ((_v) <  (_e)))

#define range(_v,_s,_e)        __range_inc(_v,_s,_e)
#define mem_range(_a,_s,_e)    __range_exc(_a,_s,_e)

#define TOP_4GB                (1ULL<<32)
#define NULL                   ((void*)0)

typedef unsigned char          uint8_t;
typedef unsigned short         uint16_t;
typedef unsigned int           uint32_t;
typedef unsigned long long     uint64_t;

typedef signed char            sint8_t;
typedef signed short           sint16_t;
typedef signed int             sint32_t;
typedef signed long long       sint64_t;

typedef char*                  string;
typedef unsigned long          size_t;
typedef unsigned long          offset_t;
typedef unsigned long          ulong_t;

typedef enum { false=0, true } bool_t;

/*
** Offset of a field from a structure
*/
#define offsetof(type,field)     ((offset_t)(&(((type*)0)->field)))

/*
** Cast pointer from value
*/
#define setptr(field, offset) ({                \
         loc_t x = { .linear = offset };        \
         field = (typeof (field))x.addr;        \
      })

/*
** Remove warnings
*/
#define __unused__                __attribute__ ((unused))

/*
** Register passing on abi-x86_64:
**  1 %rdi, 2 %rsi, 3 %rdx, 4 %rcx
*/
#define __regparm__(_n_)          __attribute__ ((regparm((_n_))))

/*
** Decomposition of 16 bis
** in 8 bits
*/
typedef union raw_16_bits_entry
{
   uint16_t raw;
   sint16_t sraw;
   uint8_t  byte[2];

   /* H/L byte access */
   struct
   {
      uint8_t blow;
      uint8_t bhigh;

   } __attribute__((packed));

} __attribute__((packed)) raw16_t;

/*
** Decomposition of 32 bis
** in 16 and 8 bits
*/
typedef union raw_32_bits_entry
{
   uint32_t raw;
   sint32_t sraw;
   uint8_t  byte[4];

   /* Full decomposition */
   struct
   {
      raw16_t _wlow;
      raw16_t _whigh;

   } __attribute__((packed));

   /* Fast acces to lower parts */
   struct
   {
      union
      {
         struct
         {
            uint8_t blow;
            uint8_t bhigh;

         } __attribute__((packed));

         uint16_t wlow;

      } __attribute__((packed));

      uint16_t whigh;

   } __attribute__((packed));

} __attribute__((packed)) raw32_t;

/*
** Decomposition of 64 bits
** in 32, 16 and 8 bits
*/
typedef union raw_64_bits_entry
{
   uint64_t raw;
   sint64_t sraw;
   uint8_t  byte[8];

   /* Full decomposition */
   struct
   {
      raw32_t _low;
      raw32_t _high;

   } __attribute__((packed));

   /* Fast access to lower parts */
   struct
   {
      union
      {
         struct
         {
            union
            {
               struct
               {
                  uint8_t blow;
                  uint8_t bhigh;

               } __attribute__((packed));

               uint16_t wlow;

            } __attribute__((packed));

            uint16_t whigh;

         } __attribute__((packed));

         uint32_t  low;

      } __attribute__((packed));

      uint32_t  high;

   } __attribute__((packed));

} __attribute__((packed)) raw64_t;

/*
** Simple far pointer structs
*/
typedef struct far_pointer
{
   raw32_t  offset;
   uint16_t segment;

} __attribute__((packed)) fptr_t;

typedef struct far_ptr16
{
   uint16_t offset;
   uint16_t segment;

} __attribute__((packed)) fptr16_t;

typedef struct far_ptr32
{
   uint32_t offset;
   uint16_t segment;

} __attribute__((packed)) fptr32_t;

typedef struct far_ptr64
{
   uint64_t offset;
   uint16_t segment;

} __attribute__((packed)) fptr64_t;

/*
** No cast on access
*/
typedef union location
{
   offset_t  linear;

   void      *addr;
   uint8_t   *u8;
   uint16_t  *u16;
   uint32_t  *u32;
   uint64_t  *u64;

   string    str;

   fptr16_t  *f16;
   fptr32_t  *f32;
   fptr64_t  *f64;

   raw64_t;

} __attribute__((packed)) loc_t;

/*
** Simple buffer struct
*/
typedef struct buffer
{
   loc_t  data;
   size_t sz;

} __attribute__((packed)) buffer_t;

/*
** Generic argument
*/
typedef loc_t arg_t;


#endif
