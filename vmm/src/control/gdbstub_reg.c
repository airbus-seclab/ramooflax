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

static offset_t __gdb_register_cache;

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

static void __gdb_reg_access_rip(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__rip);
   loc->u64 = &__rip.raw;
   __gdb_reg_access_fix_sz(size);
}

static void __gdb_reg_access_rflags(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__rflags);
   loc->u64 = &__rflags.raw;
   __gdb_reg_access_fix_sz(size);
}

static void __gdb_reg_access_cs_sel(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__cs.selector);
   loc->u16 = &__cs.selector.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_ss_sel(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ss.selector);
   loc->u16 = &__ss.selector.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_ds_sel(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ds.selector);
   loc->u16 = &__ds.selector.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_es_sel(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__es.selector);
   loc->u16 = &__es.selector.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_fs_sel(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__fs.selector);
   loc->u16 = &__fs.selector.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_gs_sel(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__gs.selector);
   loc->u16 = &__gs.selector.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_cr0(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__cr0);
   loc->u64 = &__cr0.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_cr2(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__cr2);
   loc->u64 = &__cr2.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_cr3(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__cr3);
   loc->u64 = &__cr3.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_cr4(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__cr4);
   loc->u64 = &__cr4.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_dr0(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __unused__ wr)
{
   __gdb_register_cache = get_dr0();
   loc->addr = (void*)__gdb_register_cache;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_dr1(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __unused__ wr)
{
   __gdb_register_cache = get_dr1();
   loc->addr = (void*)__gdb_register_cache;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_dr2(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __unused__ wr)
{
   __gdb_register_cache = get_dr2();
   loc->addr = (void*)__gdb_register_cache;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_dr3(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __unused__ wr)
{
   __gdb_register_cache = get_dr3();
   loc->addr = (void*)__gdb_register_cache;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_dr6(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__dr6);
   loc->u64 = &__dr6.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_dr7(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__dr7);
   loc->u64 = &__dr7.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_dbgctl(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__dbgctl);
   loc->u64 = &__dbgctl.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_efer(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __unused__ wr)
{
   loc->u64 = &__efer.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_cs_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__cs.base);
   loc->u64 = &__cs.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_ss_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ss.base);
   loc->u64 = &__ss.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_ds_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ds.base);
   loc->u64 = &__ds.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_es_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__es.base);
   loc->u64 = &__es.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_fs_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__fs.base);
   loc->u64 = &__fs.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_gs_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__gs.base);
   loc->u64 = &__gs.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_gdtr_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__gdtr.base);
   loc->u64 = &__gdtr.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_idtr_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__idtr.base);
   loc->u64 = &__idtr.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_ldtr_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ldtr.base);
   loc->u64 = &__ldtr.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_tr_base(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__tr.base);
   loc->u64 = &__tr.base.raw;
   *size = sizeof(uint64_t);
}

static void __gdb_reg_access_cs_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__cs.limit);
   loc->u32 = &__cs.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_ss_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ss.limit);
   loc->u32 = &__ss.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_ds_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ds.limit);
   loc->u32 = &__ds.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_es_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__es.limit);
   loc->u32 = &__es.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_fs_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__fs.limit);
   loc->u32 = &__fs.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_gs_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__gs.limit);
   loc->u32 = &__gs.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_gdtr_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__gdtr.limit);
   loc->u32 = &__gdtr.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_idtr_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__idtr.limit);
   loc->u32 = &__idtr.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_ldtr_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ldtr.limit);
   loc->u32 = &__ldtr.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_tr_limit(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__tr.limit);
   loc->u32 = &__tr.limit.raw;
   *size = sizeof(uint32_t);
}

static void __gdb_reg_access_cs_attr(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__cs.attributes);
   loc->u16 = (uint16_t*)&__cs.attributes.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_ss_attr(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ss.attributes);
   loc->u16 = (uint16_t*)&__ss.attributes.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_ds_attr(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ds.attributes);
   loc->u16 = (uint16_t*)&__ds.attributes.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_es_attr(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__es.attributes);
   loc->u16 = (uint16_t*)&__es.attributes.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_fs_attr(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__fs.attributes);
   loc->u16 = (uint16_t*)&__fs.attributes.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_gs_attr(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__gs.attributes);
   loc->u16 = (uint16_t*)&__gs.attributes.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_ldtr_attr(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__ldtr.attributes);
   loc->u16 = (uint16_t*)&__ldtr.attributes.raw;
   *size = sizeof(uint16_t);
}

static void __gdb_reg_access_tr_attr(
   loc_t *loc, size_t *size, uint8_t __unused__ idx, uint8_t __cused__ wr)
{
   __cond_access(wr,__tr.attributes);
   loc->u16 = (uint16_t*)&__tr.attributes.raw;
   *size = sizeof(uint16_t);
}

/*
** GDB accessors
*/
static gdb_reg_acc_e_t gdb_gpr_reg_acc[] = {
   {"rax",    __gdb_reg_access_gpr   },
   {"rcx",    __gdb_reg_access_gpr   },
   {"rdx",    __gdb_reg_access_gpr   },
   {"rbx",    __gdb_reg_access_gpr   },
   {"rsp",    __gdb_reg_access_gpr   },
   {"rbp",    __gdb_reg_access_gpr   },
   {"rsi",    __gdb_reg_access_gpr   },
   {"rdi",    __gdb_reg_access_gpr   },
   {"r8",     __gdb_reg_access_gpr   },
   {"r9",     __gdb_reg_access_gpr   },
   {"r10",    __gdb_reg_access_gpr   },
   {"r11",    __gdb_reg_access_gpr   },
   {"r12",    __gdb_reg_access_gpr   },
   {"r13",    __gdb_reg_access_gpr   },
   {"r14",    __gdb_reg_access_gpr   },
   {"r15",    __gdb_reg_access_gpr   },
   {"rip",    __gdb_reg_access_rip   },
   {"rflags", __gdb_reg_access_rflags},
   {"cs.sel", __gdb_reg_access_cs_sel},
   {"ss.sel", __gdb_reg_access_ss_sel},
   {"ds.sel", __gdb_reg_access_ds_sel},
   {"es.sel", __gdb_reg_access_es_sel},
   {"fs.sel", __gdb_reg_access_fs_sel},
   {"gs.sel", __gdb_reg_access_gs_sel},
};

static gdb_reg_acc_e_t gdb_sys_reg_acc[] = {
   {"cr0",        __gdb_reg_access_cr0       },
   {"cr2",        __gdb_reg_access_cr2       },
   {"cr3",        __gdb_reg_access_cr3       },
   {"cr4",        __gdb_reg_access_cr4       },
   {"dr0",        __gdb_reg_access_dr0       },
   {"dr1",        __gdb_reg_access_dr1       },
   {"dr2",        __gdb_reg_access_dr2       },
   {"dr3",        __gdb_reg_access_dr3       },
   {"dr6",        __gdb_reg_access_dr6       },
   {"dr7",        __gdb_reg_access_dr7       },
   {"dbgctl",     __gdb_reg_access_dbgctl    },
   {"efer",       __gdb_reg_access_efer      },
   {"cs.base",    __gdb_reg_access_cs_base   },
   {"ss.base",    __gdb_reg_access_ss_base   },
   {"ds.base",    __gdb_reg_access_ds_base   },
   {"es.base",    __gdb_reg_access_es_base   },
   {"fs.base",    __gdb_reg_access_fs_base   },
   {"gs.base",    __gdb_reg_access_gs_base   },
   {"gdtr.base",  __gdb_reg_access_gdtr_base },
   {"idtr.base",  __gdb_reg_access_idtr_base },
   {"ldtr.base",  __gdb_reg_access_ldtr_base },
   {"tr.base",    __gdb_reg_access_tr_base   },
   {"cs.limit",   __gdb_reg_access_cs_limit  },
   {"ss.limit",   __gdb_reg_access_ss_limit  },
   {"ds.limit",   __gdb_reg_access_ds_limit  },
   {"es.limit",   __gdb_reg_access_es_limit  },
   {"fs.limit",   __gdb_reg_access_fs_limit  },
   {"gs.limit",   __gdb_reg_access_gs_limit  },
   {"gdtr.limit", __gdb_reg_access_gdtr_limit},
   {"idtr.limit", __gdb_reg_access_idtr_limit},
   {"ldtr.limit", __gdb_reg_access_ldtr_limit},
   {"tr.limit",   __gdb_reg_access_tr_limit  },
   {"cs.attr",    __gdb_reg_access_cs_attr   },
   {"ss.attr",    __gdb_reg_access_ss_attr   },
   {"ds.attr",    __gdb_reg_access_ds_attr   },
   {"es.attr",    __gdb_reg_access_es_attr   },
   {"fs.attr",    __gdb_reg_access_fs_attr   },
   {"gs.attr",    __gdb_reg_access_gs_attr   },
   {"ldtr.attr",  __gdb_reg_access_ldtr_attr },
   {"tr.attr",    __gdb_reg_access_tr_attr   },
};

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
