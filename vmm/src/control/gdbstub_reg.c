/*
** Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
#include <gdbstub.h>
#include <gdbstub_reg.h>
#include <gdbstub_cmd.h>
#include <dr.h>
#include <insn.h>
#include <debug.h>
#include <info_data.h>

extern info_data_t *info;
static offset_t    __gdb_register_cache;

/*
** GDB accessor generators
*/
static void __gdb_reg_access_fix_sz(size_t *size)
{
   if(cpu_addr_sz() == 64)
      *size = sizeof(uint64_t);
   else
      *size = sizeof(uint32_t);
}

static void __gdb_reg_access_gpr(
   loc_t *loc, size_t *size, uint8_t idx, uint8_t __unused__ wr)
{
   loc->u64 = &info->vm.cpu.gpr->raw[GPR64_RAX - idx].raw;
   __gdb_reg_access_fix_sz(size);
}

#define __gen_gdb_reg_acc(_rEg_,_sZ_)					\
   static void __gdb_reg_access_##_rEg_(				\
      loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __unused__ wr) \
   {									\
      loc->u##_sZ_ = &__##_rEg_.raw;					\
      *size = sizeof(uint##_sZ_##_t);					\
   }

#define __gen_gdb_reg_acc_cond(_rEg1_,_sZ_)				\
   static void __gdb_reg_access_##_rEg1_(				\
      loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr) \
   {									\
      __cond_access(wr,__##_rEg1_);					\
      loc->u##_sZ_ = &__##_rEg1_.raw;					\
      *size = sizeof(uint##_sZ_##_t);					\
   }

#define __gen_gdb_reg_acc_cond_2(_rEg1_,_rEg2_,_sZ_)			\
   static void __gdb_reg_access_##_rEg1_##_##_rEg2_(			\
      loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr) \
   {									\
      __cond_access(wr,__##_rEg1_._rEg2_);				\
      loc->u##_sZ_ = (uint##_sZ_##_t*)&__##_rEg1_._rEg2_.raw;		\
      *size = sizeof(uint##_sZ_##_t);					\
   }

#define __gen_gdb_reg_acc_cond_fix(_rEg_,_sZ_)				\
   static void __gdb_reg_access_##_rEg_(				\
      loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr) \
   {									\
      __cond_access(wr,__##_rEg_);					\
      loc->u##_sZ_ = &__##_rEg_.raw;					\
      __gdb_reg_access_fix_sz(size);					\
   }

#define __gen_gdb_reg_acc_cache(_rEg_,_sZ_)				\
   static void __gdb_reg_access_##_rEg_(				\
      loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __unused__ wr) \
   {									\
      __gdb_register_cache = get_##_rEg_();				\
      loc->addr = (void*)&__gdb_register_cache;				\
      *size = sizeof(uint##_sZ_##_t);					\
   }

/*
** GDB internal accessors
*/
__gen_gdb_reg_acc(efer, 64)

__gen_gdb_reg_acc_cond_fix(rip,    64)
__gen_gdb_reg_acc_cond_fix(rflags, 64)

__gen_gdb_reg_acc_cond(cr0, 64)
__gen_gdb_reg_acc_cond(cr2, 64)
__gen_gdb_reg_acc_cond(cr3, 64)
__gen_gdb_reg_acc_cond(cr4, 64)

__gen_gdb_reg_acc_cache(dr0, 64)
__gen_gdb_reg_acc_cache(dr1, 64)
__gen_gdb_reg_acc_cache(dr2, 64)
__gen_gdb_reg_acc_cache(dr3, 64)

__gen_gdb_reg_acc_cond(dr6,    64)
__gen_gdb_reg_acc_cond(dr7,    64)
__gen_gdb_reg_acc_cond(dbgctl, 64)

__gen_gdb_reg_acc_cond_2(cs, selector, 16)
__gen_gdb_reg_acc_cond_2(ss, selector, 16)
__gen_gdb_reg_acc_cond_2(ds, selector, 16)
__gen_gdb_reg_acc_cond_2(es, selector, 16)
__gen_gdb_reg_acc_cond_2(fs, selector, 16)
__gen_gdb_reg_acc_cond_2(gs, selector, 16)

__gen_gdb_reg_acc_cond_2(cs,   base, 64)
__gen_gdb_reg_acc_cond_2(ss,   base, 64)
__gen_gdb_reg_acc_cond_2(ds,   base, 64)
__gen_gdb_reg_acc_cond_2(es,   base, 64)
__gen_gdb_reg_acc_cond_2(fs,   base, 64)
__gen_gdb_reg_acc_cond_2(gs,   base, 64)
__gen_gdb_reg_acc_cond_2(gdtr, base, 64)
__gen_gdb_reg_acc_cond_2(idtr, base, 64)
__gen_gdb_reg_acc_cond_2(ldtr, base, 64)
__gen_gdb_reg_acc_cond_2(tr,   base, 64)

__gen_gdb_reg_acc_cond_2(cs,   limit, 32)
__gen_gdb_reg_acc_cond_2(ss,   limit, 32)
__gen_gdb_reg_acc_cond_2(ds,   limit, 32)
__gen_gdb_reg_acc_cond_2(es,   limit, 32)
__gen_gdb_reg_acc_cond_2(fs,   limit, 32)
__gen_gdb_reg_acc_cond_2(gs,   limit, 32)
__gen_gdb_reg_acc_cond_2(gdtr, limit, 32)
__gen_gdb_reg_acc_cond_2(idtr, limit, 32)
__gen_gdb_reg_acc_cond_2(ldtr, limit, 32)
__gen_gdb_reg_acc_cond_2(tr,   limit, 32)

__gen_gdb_reg_acc_cond_2(cs,   attributes, 16)
__gen_gdb_reg_acc_cond_2(ss,   attributes, 16)
__gen_gdb_reg_acc_cond_2(ds,   attributes, 16)
__gen_gdb_reg_acc_cond_2(es,   attributes, 16)
__gen_gdb_reg_acc_cond_2(fs,   attributes, 16)
__gen_gdb_reg_acc_cond_2(gs,   attributes, 16)
__gen_gdb_reg_acc_cond_2(ldtr, attributes, 16)
__gen_gdb_reg_acc_cond_2(tr,   attributes, 16)

/*
** GDB usable register accessors
*/
static gdb_reg_acc_e_t gdb_gpr_reg_acc[] = {
   { "rax",    __gdb_reg_access_gpr         },
   { "rcx",    __gdb_reg_access_gpr         },
   { "rdx",    __gdb_reg_access_gpr         },
   { "rbx",    __gdb_reg_access_gpr         },
   { "rsp",    __gdb_reg_access_gpr         },
   { "rbp",    __gdb_reg_access_gpr         },
   { "rsi",    __gdb_reg_access_gpr         },
   { "rdi",    __gdb_reg_access_gpr         },
   { "r8",     __gdb_reg_access_gpr         },
   { "r9",     __gdb_reg_access_gpr         },
   { "r10",    __gdb_reg_access_gpr         },
   { "r11",    __gdb_reg_access_gpr         },
   { "r12",    __gdb_reg_access_gpr         },
   { "r13",    __gdb_reg_access_gpr         },
   { "r14",    __gdb_reg_access_gpr         },
   { "r15",    __gdb_reg_access_gpr         },
   { "rip",    __gdb_reg_access_rip         },
   { "rflags", __gdb_reg_access_rflags      },
   { "cs.sel", __gdb_reg_access_cs_selector },
   { "ss.sel", __gdb_reg_access_ss_selector },
   { "ds.sel", __gdb_reg_access_ds_selector },
   { "es.sel", __gdb_reg_access_es_selector },
   { "fs.sel", __gdb_reg_access_fs_selector },
   { "gs.sel", __gdb_reg_access_gs_selector }};

static gdb_reg_acc_e_t gdb_sys_reg_acc[] = {
   { "cr0",        __gdb_reg_access_cr0             },
   { "cr2",        __gdb_reg_access_cr2             },
   { "cr3",        __gdb_reg_access_cr3             },
   { "cr4",        __gdb_reg_access_cr4             },
   { "dr0",        __gdb_reg_access_dr0             },
   { "dr1",        __gdb_reg_access_dr1             },
   { "dr2",        __gdb_reg_access_dr2             },
   { "dr3",        __gdb_reg_access_dr3             },
   { "dr6",        __gdb_reg_access_dr6             },
   { "dr7",        __gdb_reg_access_dr7             },
   { "dbgctl",     __gdb_reg_access_dbgctl          },
   { "efer",       __gdb_reg_access_efer            },
   { "cs.base",    __gdb_reg_access_cs_base         },
   { "ss.base",    __gdb_reg_access_ss_base         },
   { "ds.base",    __gdb_reg_access_ds_base         },
   { "es.base",    __gdb_reg_access_es_base         },
   { "fs.base",    __gdb_reg_access_fs_base         },
   { "gs.base",    __gdb_reg_access_gs_base         },
   { "gdtr.base",  __gdb_reg_access_gdtr_base       },
   { "idtr.base",  __gdb_reg_access_idtr_base       },
   { "ldtr.base",  __gdb_reg_access_ldtr_base       },
   { "tr.base",    __gdb_reg_access_tr_base         },
   { "cs.limit",   __gdb_reg_access_cs_limit        },
   { "ss.limit",   __gdb_reg_access_ss_limit        },
   { "ds.limit",   __gdb_reg_access_ds_limit        },
   { "es.limit",   __gdb_reg_access_es_limit        },
   { "fs.limit",   __gdb_reg_access_fs_limit        },
   { "gs.limit",   __gdb_reg_access_gs_limit        },
   { "gdtr.limit", __gdb_reg_access_gdtr_limit      },
   { "idtr.limit", __gdb_reg_access_idtr_limit      },
   { "ldtr.limit", __gdb_reg_access_ldtr_limit      },
   { "tr.limit",   __gdb_reg_access_tr_limit        },
   { "cs.attr",    __gdb_reg_access_cs_attributes   },
   { "ss.attr",    __gdb_reg_access_ss_attributes   },
   { "ds.attr",    __gdb_reg_access_ds_attributes   },
   { "es.attr",    __gdb_reg_access_es_attributes   },
   { "fs.attr",    __gdb_reg_access_fs_attributes   },
   { "gs.attr",    __gdb_reg_access_gs_attributes   },
   { "ldtr.attr",  __gdb_reg_access_ldtr_attributes },
   { "tr.attr",    __gdb_reg_access_tr_attributes   }};

#define gdb_gpr_reg_acc_nr (sizeof(gdb_gpr_reg_acc)/sizeof(gdb_reg_acc_e_t))
#define gdb_sys_reg_acc_nr (sizeof(gdb_sys_reg_acc)/sizeof(gdb_reg_acc_e_t))

static gdb_reg_acc_t gdb_reg_accessor[] = {
   {gdb_gpr_reg_acc, gdb_gpr_reg_acc_nr},
   {gdb_sys_reg_acc, gdb_sys_reg_acc_nr},
};

int __gdb_setup_reg(uint64_t idx,raw64_t **reg,size_t *size,uint8_t sys,uint8_t wr)
{
   loc_t loc;

   sys %= 2;

   if(!sys && cpu_addr_sz() != 64 && idx > 7)
      idx += 8;

   if(idx > gdb_reg_accessor[sys].nr)
   {
      debug(GDBSTUB_PKT, "reg_op invalid index %d\n", idx);
      return 0;
   }

   debug(GDBSTUB_PKT, "reg_op %s\n", gdb_reg_accessor[sys].reg[idx].name);

   gdb_reg_accessor[sys].reg[idx].handler(&loc, size, idx, wr);
   *reg = (raw64_t*)loc.u64;
   return 1;
}
