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
#include <mbi.h>
#include <string.h>
#include <debug.h>

static int __mbi_parse_opt(char **pstr, char *wopt, mbi_opt_hdl_t hdl, void *res)
{
   char *str, *val, *opt, *oe, *ve;
   char oec, vec;
   int  rc = 1;

   str = *pstr;
   oe = ve = val = 0;
   opt = str++; /* get name */

   /* get value */
   for( ; *str && *str != '=' && *str != ' '; str++);

   if(*str == '=')
   {
      oe = str; oec = *oe;
      *str++ = '\x00';
      val = str;

      for( ; *str && *str != ' ' ; str++);

      /* if more options terminates value */
      if(*str)
      {
         ve = str; vec = *ve;
         *str++ = '\x00';
      }
   }
   else if(*str == ' ')
   {
      oe = str; oec = *oe;
      *str++ = '\x00';
   }

   /* check name */
   for( ; *wopt && *opt && *wopt == *opt ; wopt++, opt++);

   if(*wopt || *opt)
      goto __bad_opt;

   if(val)
      rc = hdl(val, res);

__leave:
   if(oe && *oe != oec) *oe = oec;
   if(ve && *ve != vec) *ve = vec;

   *pstr = str;
   return rc;

__bad_opt:
   rc = 0;
   goto __leave;
}

void mbi_check_boot_loader(mbi_t *mbi)
{
   char *str;

   if(!(mbi->flags & MBI_FLAG_BLDR))
   {
      debug(MBI, "no bootloader name provided, assume GRUB 2\n");
      return;
   }

   str = (char*)((offset_t)mbi->boot_loader_name);
   while(*str)
   {
      if(*str != 'G' || *(uint32_t*)str != GRUB_STR || str[5] != '0')
      {
         str++;
         continue;
      }

      /* fake flag to mark we detected GRUB legacy (0.9x) */
      mbi->flags |= MBI_FLAG_VER;
      return;
   }
}

int mbi_get_opt(mbi_t *mbi, module_t *mod, char *opt, mbi_opt_hdl_t hdl, void *res)
{
   int  rc;
   char *str = (char*)((offset_t)mod->cmdline);

   debug(MBI, "mbi getopt \"%s\" on %s\n", opt, str);

   /* GRUB 1 keeps module name */
   if(mbi->flags & MBI_FLAG_VER)
      for( ; *str && *str != ' ' ; str++);

   do for( ; *str && *str == ' ' ; str++);
   while(!(rc=__mbi_parse_opt(&str, opt, hdl, res)) && *str);

   return rc;
}
