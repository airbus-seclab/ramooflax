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
