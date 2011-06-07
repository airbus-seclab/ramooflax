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
#ifndef __VMX_CPUID_H__
#define __VMX_CPUID_H__

#include <types.h>

/*
** CPUID Processor Features
*/
#define CPUID_ECX_FEAT_VMX_BIT          5
#define CPUID_ECX_FEAT_VMX              (1<<CPUID_ECX_FEAT_VMX_BIT)

#define vmx_supported()				\
   ({						\
      uint32_t c=0,d;				\
      cpuid_features(c,d);			\
      (c & CPUID_ECX_FEAT_VMX)?1:0;		\
   })

/*
** CPUID Extended Processor Features
*/
#define check_cpu_skillz()						\
   ({									\
      uint32_t c=0, d, ed;						\
      uint32_t feat =							\
	 CPUID_EDX_FEAT_PSE |						\
	 CPUID_EDX_FEAT_PAE |						\
	 CPUID_EDX_FEAT_PAT;						\
      uint32_t efeat =							\
	 CPUID_EDX_EXT_PROC_FEAT_LM;					\
      cpuid_features(c,d);						\
      cpuid_ext_proc_feat(c,ed);					\
      if((( d &  feat) !=  feat) ||					\
	 ((ed & efeat) != efeat))					\
	 panic("cpu not skilled enough: pse %d pae %d pat %d lm %d",	\
	       (d&CPUID_EDX_FEAT_PSE)?1:0,				\
	       (d&CPUID_EDX_FEAT_PAE)?1:0,				\
	       (d&CPUID_EDX_FEAT_PAT)?1:0,				\
	       (ed&CPUID_EDX_EXT_PROC_FEAT_LM)?1:0			\
	    );								\
   })

#endif
