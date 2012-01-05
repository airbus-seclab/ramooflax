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
#include <mbi.h>
#include <string.h>

int __mbi_parse_opt(char **pstr, char *wopt, mbi_opt_hdl_t hdl, void *res)
{
   char *str = *pstr, *opt, *val=0;
   int  rc = 1;

   /* get name */
   opt = str++;

   /* get value */
   for( ; *str && *str != '=' && *str != ' '; str++);

   if(*str == '=')
   {
      *str++ = '\x00';
      val = str;

      for( ; *str && *str != ' ' ; str++);

      /* if more options terminates value */
      if(*str)
	 *str++ = '\x00';
   }
   else if(*str == ' ')
      *str++ = '\x00';

   /* check name */
   for( ; *wopt && *opt && *wopt == *opt ; wopt++, opt++);

   if(*wopt || *opt)
      goto __bad_opt;

   if(val)
      rc = hdl(val, res);

__leave:
   *pstr = str;
   return rc;

__bad_opt:
   rc = 0;
   goto __leave;
}

int mbi_get_opt(module_t *mod, char *opt, mbi_opt_hdl_t hdl, void *res)
{
   int  rc;
   char *str = (char*)((offset_t)mod->cmdline);

   /* ignore module name */
   for( ; *str && *str != ' ' ; str++);

   do for( ; *str && *str == ' ' ; str++);
   while(!(rc=__mbi_parse_opt(&str, opt, hdl, res)) && *str);

   return rc;
}
