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
#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>
#include <gpr.h>
#include <math.h>

/*
** Size is number of store operations to repeat
*/
#define __fix_str_dir(__op__,a,b,c,l)		\
   ({						\
      xflags_reg_t save;			\
      save_flags(save);				\
      asm volatile ("cld");			\
      __op__(a,b,c,l);				\
      load_flags(save);				\
   })


#define  __rep_8_16(b) ((((uint16_t)b)<<8)|((uint16_t)b))
#define  __rep_8_32(b) ((((uint32_t)__rep_8_16(b))<<16)|((uint32_t)__rep_8_16(b)))

#define __replicate_byte_on_word(b)  __rep_8_16(b)
#define __replicate_byte_on_dword(b) __rep_8_32(b)

#ifdef __X86_64__

#define __rep_8_64(b)  ((((uint64_t)__rep_8_32(b))<<32)|((uint64_t)__rep_8_32(b)))
#define __replicate_byte_on_long(b) __rep_8_64(b)
#define __memset(d,v,t,l)   asm volatile ("rex.w rep stosl"::"D"(d),"a"(v),"c"(t))
#define __memcpy(d,s,t,l)   asm volatile ("rex.w rep movsl"::"D"(d),"S"(s),"c"(t))

#else

#define __replicate_byte_on_long(b) __rep_8_32(b)
#define __memset(d,v,t,l)   asm volatile ("rep stosl"::"D"(d),"a"(v),"c"(t))
#define __memcpy(d,s,t,l)   asm volatile ("rep movsl"::"D"(d),"S"(s),"c"(t))

#endif


#define _memset(d,v,t)      __fix_str_dir(__memset,d,v,t,0)
#define _memcpy(d,s,t)      __fix_str_dir(__memcpy,d,s,t,0)

#define __memset8(d,v,t,l)  asm volatile ("rep stosb"::"D"(d),"a"(v),"c"(t))
#define __memcpy8(d,s,t,l)  asm volatile ("rep movsb"::"D"(d),"S"(s),"c"(t))
#define __memchr8(d,s,v,l)  asm volatile ("repnz scasb":"=D"(d), "=c"(l):"D"(s),"a"(v),"c"(l))

#define _memset8(d,v,t)     __fix_str_dir(__memset8,d,v,t,0)
#define _memcpy8(d,s,t)     __fix_str_dir(__memcpy8,d,s,t,0)
#define _memchr8(d,s,v,l)   __fix_str_dir(__memchr8,d,s,v,l)

/*
** Size is number of bytes
*/
static inline void* memset(void *dst, uint8_t c, size_t size)
{
   size_t cnt, rm;
   loc_t  dloc;

   dloc.addr = dst;

   if(!size)
      return dst;

   __divrm(size, sizeof(ulong_t), cnt, rm);

   if(cnt)
   {
      ulong_t lc = __replicate_byte_on_long(c);
      _memset(dloc.linear, lc, cnt);
      dloc.linear += cnt*sizeof(ulong_t);
   }

   if(rm)
      _memset8(dloc.linear, c, rm);

   return dst;
}

static inline void* memcpy(void *dst, void *src, size_t size)
{
   size_t cnt, rm;
   loc_t  dloc, sloc;

   dloc.addr = dst;
   sloc.addr = src;

   if(!size)
      return dst;

   __divrm(size, sizeof(ulong_t), cnt, rm);

   if(cnt)
   {
      _memcpy(dloc.linear, sloc.linear, cnt);
      dloc.linear += cnt*sizeof(ulong_t);
      sloc.linear += cnt*sizeof(ulong_t);
   }

   if(rm)
      _memcpy8(dloc.linear, sloc.linear, rm);

   return dst;
}

static inline char* strchr(char *str, size_t len, char c)
{
   loc_t d, s;

   s.str = str;
   _memchr8(d.linear, s.linear, c, len);

   if(!len)
      return (char*)0;

   return (d.str-1);
}

static inline size_t strlen(char *str)
{
   loc_t  d, s;
   size_t len = -1UL;

   s.str = str;
   _memchr8(d.linear, s.linear, 0, len);
   return ((d.linear-1) - s.linear);
}

static inline void __buf_add(buffer_t *buf, size_t len, char c)
{
   if(buf->sz < len)
      buf->data.str[buf->sz++] = c;
}

#define BAD_NIBBLE ((uint8_t)-1)

/*
** Prototypes
*/
int    dec_to_uint64(uint8_t*, size_t, uint64_t*);
int    hex_to_uint64(uint8_t*, size_t, uint64_t*);
int    __hex_to_uint8(uint8_t*, uint8_t*);

size_t uint64_to_hex(buffer_t*, size_t, uint64_t, size_t);
void   __uint8_to_hex(uint8_t*, uint8_t);

#endif
