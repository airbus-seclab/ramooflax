#
# Copyright (C) 2011 EADS France, stephane duverger <stephane.duverger@eads.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
class IdtEvent:
    HW_INT  = 0
    NMI     = 2
    HW_EXCP = 3
    SW_INT  = 4

    def __init__(self, raw):
        self.vector   =  raw & 0xff
        self.type     = (raw>>8)  & 7
        self.has_err  = (raw>>11) & 1
        self.v        = (raw>>31) & 1
        self.err      = (raw>>32) & 0xffffffff

    def __str__(self):
        return repr(self)

    def __repr__(self):
        hdr = "IDT Event : "
        if not self.v:
            return hdr+"None"

        if self.has_err:
            errs = hex(self.err)
        else:
            errs = "None"

        return hdr+"- vector  : %d\n- type    : %d\n- err     : %s\n" % \
            (self.vector, self.type, err_s)

class StopReason:
    every      =  0 # any stop reason

    gdb_int    =  2 # keyboard interrupt (^c)
    gdb_trap   =  5 # breakpoints, singlestep

    rd_cr0     = 10
    rd_cr2     = 12
    rd_cr3     = 13
    rd_cr4     = 14

    wr_cr0     = 20
    wr_cr2     = 22
    wr_cr3     = 23
    wr_cr4     = 24

    excp_de    = 30
    excp_db    = 31
    excp_nmi   = 32
    excp_bp    = 33
    excp_of    = 34
    excp_br    = 35
    excp_ud    = 36
    excp_nm    = 37
    excp_df    = 38
    excp_mo    = 39
    excp_ts    = 40
    excp_np    = 41
    excp_ss    = 42
    excp_gp    = 43
    excp_pf    = 44
    excp_rsvd  = 45
    excp_mf    = 46
    excp_ac    = 47
    excp_mc    = 48
    excp_xf    = 49

    hard_int   = 50
    soft_int   = 51
    npf        = 52

    def __init__(self, r, m=None, d=None):
        self.__s = {
            2:"gdb_int",
            5:"gdb_trap",
            10:"rd_cr0",
            12:"rd_cr2",
            13:"rd_cr3",
            14:"rd_cr4",
            20:"wr_cr0",
            22:"wr_cr2",
            23:"wr_cr3",
            24:"wr_cr4",
            30:"excp_de",
            31:"excp_db",
            32:"excp_nmi",
            33:"excp_bp",
            34:"excp_of",
            35:"excp_br",
            36:"excp_ud",
            37:"excp_nm",
            38:"excp_df",
            39:"excp_mo",
            40:"excp_ts",
            41:"excp_np",
            42:"excp_ss",
            43:"excp_gp",
            44:"excp_pf",
            45:"excp_rsvd",
            46:"excp_mf",
            47:"excp_ac",
            48:"excp_mc",
            49:"excp_xf",
            50:"hard_int",
            51:"soft_int",
            52:"npf",
            }

        self.reason = r
        self.mode = m
        self.msg = d

    def __str__(self):
        return self.__s.get(self.reason, "Unknown")

    def __len__(self):
        return len(self.msg)

class EventFilter:
    def __init__(self, dico=None):
        self.__handlers = {StopReason.every:lambda x: True}
        if dico is not None:
            self.__handlers.update(dico)

    def __call__(self, reason, vm):
        self.__handlers[StopReason.every](vm)
        if self.__handlers.has_key(reason):
            return self.__handlers[reason](vm)
        return True #default can interact

    def register(self, reason, handler):
        self.__handlers.update({reason:handler})

    def get(self, reason):
        if self.__handlers.has_key(reason):
            return self.__handlers[reason]
        return None

    def unregister(self, reason):
        if self.__handlers.has_key(reason):
            self.__handlers.pop(reason)
