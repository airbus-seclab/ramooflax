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
#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <print.h>

#define __ENABLE_DEBUG__

#ifdef  __ENABLE_DEBUG__

/*
** INIT debugging (video output)
*/
#ifdef __INIT__

#define VMM_DBG
#define MP_DBG
//#define PG_DBG
//#define SMAP_DBG
#define PMEM_DBG
#define CPU_DBG
#define EHCI_DBG
#define UART_DBG


#else
/*
** VMM debugging (uart/ehci output)
*/
#define DEBUG_VMEXIT_TRACE

#define VMM_DBG
#define MP_DBG
//#define PG_DBG

#if !defined(__EHCI_PRINT__)
#define EHCI_DBG
#endif

#if !defined(__UART_PRINT__)
#define UART_DBG
#endif

#define EMU_DBG
#define DIS_DBG
//#define EMU_INSN_DBG
//#define DIS_INSN_DBG
//#define PVL_DBG
#define INSN_DBG
#define CPUID_DBG
#define CR_DBG
#define DR_DBG
#define MSR_DBG
#define IRQ_DBG
#define INT_DBG
#define IO_DBG
#define EXCP_DBG
#define GP_DBG
#define BP_DBG
#define DB_DBG
#define PF_DBG
//#define PF_VERBOSE_DBG
#define SMI_DBG
#define CTRL_DBG

#define GDB_DBG
#define GDB_PKT_DBG
#define GDB_CMD_DBG
//#define GDB_PARSE_DBG

//#define DEV_DBG
//#define DEV_PIC_DBG
//#define DEV_UART_DBG
//#define DEV_IO_DBG
//#define DEV_PS2_DBG
//#define DEV_KBD_DBG

#define SVM_DBG
#define SVM_IDT_DBG
#define SVM_EXCP_DBG
#define SVM_EXCP_GP_DBG
#define SVM_EXCP_PF_DBG
#define SVM_NPF_DBG
#define SVM_INT_DBG
#define SVM_IO_DBG
#define SVM_IRQ_DBG
#define SVM_CPUID_DBG
#define SVM_MSR_DBG
#define SVM_CR_DBG

//#define VMX_DBG
//#define VMX_IDT_DBG
//#define VMX_EXCP_DBG
//#define VMX_EXCP_PF_DBG
//#define VMX_EXCP_GP_DBG
//#define VMX_INT_DBG
//#define VMX_IO_DBG
//#define VMX_IRQ_DBG
//#define VMX_CPUID_DBG
//#define VMX_MSR_DBG
//#define VMX_CR_DBG

#endif
#endif

/*
** Macros
*/
#if defined VMM_DBG
#define __DEBUG_VMM_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMM_DBG(fmt,fct,...) ({})
#endif

#if defined MP_DBG
#define __DEBUG_MP_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_MP_DBG(fmt,fct,...) ({})
#endif

#if defined SMAP_DBG
#define __DEBUG_SMAP_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SMAP_DBG(fmt,fct,...) ({})
#endif

#if defined PMEM_DBG
#define __DEBUG_PMEM_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_PMEM_DBG(fmt,fct,...) ({})
#endif

#if defined CPU_DBG
#define __DEBUG_CPU_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_CPU_DBG(fmt,fct,...) ({})
#endif

#if defined EHCI_DBG
#define __DEBUG_EHCI_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_EHCI_DBG(fmt,fct,...) ({})
#endif

#if defined UART_DBG
#define __DEBUG_UART_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_UART_DBG(fmt,fct,...) ({})
#endif

#if defined PG_DBG
#define __DEBUG_PG_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_PG_DBG(fmt,fct,...) ({})
#endif

#if defined EMU_DBG
#define __DEBUG_EMU_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_EMU_DBG(fmt,fct,...) ({})
#endif

#if defined EMU_INSN_DBG
#define __DEBUG_EMU_INSN_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_EMU_INSN_DBG(fmt,fct,...) ({})
#endif

#if defined DIS_DBG
#define __DEBUG_DIS_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DIS_DBG(fmt,fct,...) ({})
#endif

#if defined DIS_INSN_DBG
#define __DEBUG_DIS_INSN_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DIS_INSN_DBG(fmt,fct,...) ({})
#endif

#if defined PVL_DBG
#define __DEBUG_PVL_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_PVL_DBG(fmt,fct,...) ({})
#endif

#if defined INSN_DBG
#define __DEBUG_INSN_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_INSN_DBG(fmt,fct,...) ({})
#endif

#if defined CPUID_DBG
#define __DEBUG_CPUID_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_CPUID_DBG(fmt,fct,...) ({})
#endif

#if defined CR_DBG
#define __DEBUG_CR_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_CR_DBG(fmt,fct,...) ({})
#endif

#if defined DR_DBG
#define __DEBUG_DR_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DR_DBG(fmt,fct,...) ({})
#endif

#if defined MSR_DBG
#define __DEBUG_MSR_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_MSR_DBG(fmt,fct,...) ({})
#endif

#if defined IRQ_DBG
#define __DEBUG_IRQ_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_IRQ_DBG(fmt,fct,...) ({})
#endif

#if defined EXCP_DBG
#define __DEBUG_EXCP_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_EXCP_DBG(fmt,fct,...) ({})
#endif

#if defined INT_DBG
#define __DEBUG_INT_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_INT_DBG(fmt,fct,...) ({})
#endif

#if defined IO_DBG
#define __DEBUG_IO_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_IO_DBG(fmt,fct,...) ({})
#endif

#if defined PF_DBG
#define __DEBUG_PF_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_PF_DBG(fmt,fct,...) ({})
#endif

#if defined PF_VERBOSE_DBG
#define __DEBUG_PF_VERBOSE_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_PF_VERBOSE_DBG(fmt,fct,...) ({})
#endif

#if defined GP_DBG
#define __DEBUG_GP_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_GP_DBG(fmt,fct,...) ({})
#endif

#if defined BP_DBG
#define __DEBUG_BP_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_BP_DBG(fmt,fct,...) ({})
#endif

#if defined DB_DBG
#define __DEBUG_DB_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DB_DBG(fmt,fct,...) ({})
#endif

#if defined SMI_DBG
#define __DEBUG_SMI_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SMI_DBG(fmt,fct,...) ({})
#endif

#if defined CTRL_DBG
#define __DEBUG_CTRL_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_CTRL_DBG(fmt,fct,...) ({})
#endif

#if defined GDB_DBG
#define __DEBUG_GDB_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_GDB_DBG(fmt,fct,...) ({})
#endif

#if defined GDB_PKT_DBG
#define __DEBUG_GDB_PKT_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_GDB_PKT_DBG(fmt,fct,...) ({})
#endif

#if defined GDB_CMD_DBG
#define __DEBUG_GDB_CMD_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_GDB_CMD_DBG(fmt,fct,...) ({})
#endif

#if defined GDB_PARSE_DBG
#define __DEBUG_GDB_PARSE_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_GDB_PARSE_DBG(fmt,fct,...) ({})
#endif

#if defined DEV_DBG
#define __DEBUG_DEV_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DEV_DBG(fmt,fct,...) ({})
#endif

#if defined DEV_PIC_DBG
#define __DEBUG_DEV_PIC_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DEV_PIC_DBG(fmt,fct,...) ({})
#endif

#if defined DEV_UART_DBG
#define __DEBUG_DEV_UART_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DEV_UART_DBG(fmt,fct,...) ({})
#endif

#if defined DEV_IO_DBG
#define __DEBUG_DEV_IO_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DEV_IO_DBG(fmt,fct,...) ({})
#endif

#if defined DEV_PS2_DBG
#define __DEBUG_DEV_PS2_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DEV_PS2_DBG(fmt,fct,...) ({})
#endif

#if defined DEV_KBD_DBG
#define __DEBUG_DEV_KBD_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_DEV_KBD_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_DBG
#define __DEBUG_SVM_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_IDT_DBG
#define __DEBUG_SVM_IDT_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_IDT_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_EXCP_DBG
#define __DEBUG_SVM_EXCP_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_EXCP_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_EXCP_GP_DBG
#define __DEBUG_SVM_EXCP_GP_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_EXCP_GP_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_EXCP_PF_DBG
#define __DEBUG_SVM_EXCP_PF_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_EXCP_PF_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_NPF_DBG
#define __DEBUG_SVM_NPF_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_NPF_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_INT_DBG
#define __DEBUG_SVM_INT_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_INT_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_IO_DBG
#define __DEBUG_SVM_IO_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_IO_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_IRQ_DBG
#define __DEBUG_SVM_IRQ_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_IRQ_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_CPUID_DBG
#define __DEBUG_SVM_CPUID_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_CPUID_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_MSR_DBG
#define __DEBUG_SVM_MSR_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_MSR_DBG(fmt,fct,...) ({})
#endif

#if defined SVM_CR_DBG
#define __DEBUG_SVM_CR_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_SVM_CR_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_DBG
#define __DEBUG_VMX_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_IDT_DBG
#define __DEBUG_VMX_IDT_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_IDT_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_EXCP_DBG
#define __DEBUG_VMX_EXCP_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_EXCP_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_EXCP_GP_DBG
#define __DEBUG_VMX_EXCP_GP_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_EXCP_GP_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_EXCP_PF_DBG
#define __DEBUG_VMX_EXCP_PF_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_EXCP_PF_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_INT_DBG
#define __DEBUG_VMX_INT_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_INT_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_IO_DBG
#define __DEBUG_VMX_IO_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_IO_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_IRQ_DBG
#define __DEBUG_VMX_IRQ_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_IRQ_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_CPUID_DBG
#define __DEBUG_VMX_CPUID_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_CPUID_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_MSR_DBG
#define __DEBUG_VMX_MSR_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_MSR_DBG(fmt,fct,...) ({})
#endif

#if defined VMX_CR_DBG
#define __DEBUG_VMX_CR_DBG(fmt,fct,...) ({fct(fmt, ## __VA_ARGS__);})
#else
#define __DEBUG_VMX_CR_DBG(fmt,fct,...) ({})
#endif

/*
** Debug macro
*/
#ifndef __INIT__
#ifdef DEBUG_VMEXIT_TRACE
#ifdef __SVM__
#define __exit_code__  info->vm.cpu.vmc->vm_vmcb.ctrls_area.exit_code.low
#else
#define __exit_code__  info->vm.vmcs.exit_info.reason.raw
#endif

#define debug_defined
#define debug(who, format, ...)  __DEBUG_##who##_DBG(			\
      format,								\
      printf("<0x%X:0x%X:%d>"						\
	     , info->vmm.ctrl.vmexit_cnt.raw				\
	     , __cs.base_addr.raw + __rip.raw				\
	     , __exit_code__						\
	 );								\
     printf, ## __VA_ARGS__)
#endif
#endif

#ifndef debug_defined
#define debug_defined
#define debug(who, format, ...)  __DEBUG_##who##_DBG(format,printf, ## __VA_ARGS__)
#endif

#define debug_warning()  ({offset_t WARNING_PATCH_CODE_HERE;})

#endif
