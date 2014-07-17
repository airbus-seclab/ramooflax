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
#include <print.h>
#include <string.h>
#include <stack_trace.h>
#include <asm.h>

#ifndef __INIT__
#include <info_data.h>
extern info_data_t *info;
#endif

static char vprint_buffer[1024];

void __panic(const char *fname, const char *format, ...)
{
   va_list params;

   force_interrupts_off();

   stack_trace();

   printf("\n\n:: EVIL :: %s() :: ", fname);

   va_start(params, format);
   __vprintf(format, params);
   va_end(params);

   debug_flush();
   lock_vmm();
}

size_t printf(const char *format, ...)
{
   va_list params;
   size_t  retval;

   va_start(params, format);
   retval = __vprintf(format, params);
   va_end(params);

   return retval;
}

size_t snprintf(char *buff, size_t len, const char *format, ...)
{
   va_list params;
   size_t  retval;

   va_start(params, format);
   retval = __vsnprintf(buff, len, format, params);
   va_end(params);

   return retval;
}

size_t __vprintf(const char *format, va_list params)
{
   size_t retval;

   retval = __vsnprintf(vprint_buffer, sizeof(vprint_buffer), format, params);
   debug_write((uint8_t*)vprint_buffer, retval-1);
   return retval;
}

static inline void __format_add_str(buffer_t *buf, size_t len, char *s)
{
   while(*s)
      __buf_add(buf, len, *s++);
}

static inline void __format_add_chr(buffer_t *buf, size_t len, int c)
{
   __buf_add(buf, len, (char)c);
}

static inline void __format_add_bin(buffer_t *buf, size_t len, uint64_t value, uint32_t n)
{
   uint32_t i, bit;

   for(i=0 ; i<n ; i++)
   {
      bit = value & (1<<(n-i));
      __buf_add(buf, len, bit?'1':'0');
   }
}

static inline void __format_add_dec(buffer_t *buf, size_t len, sint64_t value)
{
   char     rep[24];
   buffer_t dec;

   if(!value)
      return __buf_add(buf, len, '0');

   dec.data.str = rep;
   dec.sz = 0;

   if(value < 0)
   {
      __buf_add(buf, len, '-');
      value = -value;
   }

   while(value)
   {
      dec.data.str[dec.sz++] = (value%10) + '0';
      value /= 10;
   }

   while(dec.sz--)
      __buf_add(buf, len, dec.data.str[dec.sz]);
}

static inline void __format_add_hex(buffer_t *buf, size_t len,
				    uint64_t value, size_t precision)
{
   uint64_to_hex(buf, len, value, precision);
}

size_t __vsnprintf(char *buffer, size_t len, const char *format, va_list params)
{
   buffer_t buf;
   char     c;
   bool_t   interp;

   buf.data.str = buffer;
   buf.sz = 0;
   interp = false;

   if(len) len--;

   while(*format)
   {
      c = *format++;

      if(interp)
      {
	 if(c == 's'){
	    char* value = va_arg(params, char*);
	    __format_add_str(&buf, len, value);
	 } else if(c == 'c'){
	    int value = va_arg(params, int);
	    __format_add_chr(&buf, len, value);
	 } else if(c == 'b'){
	    uint64_t value = va_arg(params, uint32_t);
	    __format_add_bin(&buf, len, value, 32);
	 } else if(c == 'B'){
	    uint64_t value = va_arg(params, uint64_t);
	    __format_add_bin(&buf, len, value, 64);
	 } else if(c == 'd'){
	    sint64_t value = va_arg(params, sint32_t);
	    __format_add_dec(&buf, len, value);
	 } else if(c == 'D'){
	    sint64_t value = va_arg(params, sint64_t);
	    __format_add_dec(&buf, len, value);
	 } else if(c == 'x'){
	    uint64_t value = va_arg(params, uint32_t);
	    __format_add_hex(&buf, len, value, 0);
	 } else if(c == 'X'){
	    uint64_t value = va_arg(params, uint64_t);
	    __format_add_hex(&buf, len, value, 0);
	 } else {
	    __buf_add(&buf, len, c);
	 }
	 interp = false;
      }
      else if(c == '%')
	 interp = true;
      else
         __buf_add(&buf, len, c);
   }

   buf.data.str[buf.sz++] = 0;
   return buf.sz;
}
