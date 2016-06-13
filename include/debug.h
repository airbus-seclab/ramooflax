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
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <print.h>
#include <config.h>
#include <config_debug.h>

#define debug_warning()  ({offset_t WARNING_PATCH_CODE_HERE;})

#ifndef __X86_64__
#define debug(who, format, ...)  ({})
#else
/*
** Prevent self debugging
*/
#if !defined __INIT__ && defined(CONFIG_PRINT_EHCI) && defined(CONFIG_EHCI_DBG)
#undef CONFIG_EHCI_DBG
#endif

#if !defined __INIT__ && defined(CONFIG_PRINT_UART) && defined(CONFIG_UART_DBG)
#undef CONFIG_UART_DBG
#endif

#if !defined __INIT__ && defined(CONFIG_PRINT_NET) && defined(CONFIG_NET_DBG)
#undef CONFIG_NET_DBG
#endif

/*
** Debug macro
*/
#ifndef __INIT__
#ifdef CONFIG_VMEXIT_TRACE
#ifdef CONFIG_ARCH_AMD
#define __exit_code__  vm_ctrls.exit_code.low
#else
#define __exit_code__  vm_exit_info.reason.basic
#endif

#define debug_defined
#define debug(who, format, ...)                 \
   ({                                           \
      offset_t _dbg_loc_;                               \
      int      _dbg_md_;                                \
      vm_get_code_addr(&_dbg_loc_, 0, &_dbg_md_);       \
      DEBUG_CONFIG_##who##_DBG(                         \
         format,                                        \
         printf("0x%X:%d:0x%X:%d:"                      \
                ,info->vmm.ctrl.vmexit_cnt.raw          \
                ,__exit_code__                          \
                ,_dbg_loc_,_dbg_md_                     \
            );                                          \
         printf, ## __VA_ARGS__);                       \
   })

#endif
#endif

#ifndef debug_defined
#define debug_defined
#define debug(who, format, ...)  \
   DEBUG_CONFIG_##who##_DBG(format,printf, ## __VA_ARGS__)
#endif

#endif
#endif
