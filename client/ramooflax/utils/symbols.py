#
# Copyright (C) 2016 Airbus Group, stephane duverger <stephane.duverger@airbus.com>
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
from ramooflax.core import log

class Symbol:
    def __init__(self, addr, typ, name, size='0'):
        self.name = name
        self.addr = int(addr,16)
        self.size = int(size,16)

        if typ == 't' or typ == 'T' or typ == 'FUNC':
            self.type = 'f'
        else:
            self.type = typ

    def __cmp__(self, v):
        if self.addr < v.addr:
            return -1
        if self.addr == v.addr:
            return 0
        return 1

    def __str__(self):
        return repr(self)

    def __repr__(self):
        return "%s [0x%.8x 0x%.8x] (%d)" % \
            (self.name, self.addr, self.addr+self.size, self.size)

class SymParser:
    def __init__(self):
        self.last = Symbol('0xffffffff','FUNC','__NO_SUCH_SYMBOL__','0xffffffff')
        self.__symbols = [self.last]

    def __start(self):
        log("symbols", "loading symbols")

    def __finish(self):
        if len(self.__symbols) == 1:
            return None

        log("symbols", "sorting symbols")
        self.__symbols.sort()

        log("symbols", "recompute size")
        for i in range(0,len(self.__symbols)):
            s = self.__symbols[i]
            s.prev = self.__symbols[(i-1)%len(self.__symbols)]
            s.next = self.__symbols[(i+1)%len(self.__symbols)]
            if s.size == 0:
                n = s.next
                s.size = n.addr - s.addr

        return self.__symbols

    # Linux kernel system map, default "nm" output parser
    def from_system_map(self, f):
        self.__start()
        for s in open(f).readlines():
            sym = Symbol(*s[:-1].split())
            self.__symbols.append(sym)

        return self.__finish()

    # "nm -f sysv" output parser
    def from_nm_sysv(self, f):
        self.__start()
        return self.__finish()

class SymTab:
    def __init__(self, symbols):
        self.__symbols = symbols
        self.__asymbols = {}
        self.__nsymbols = {}

        for i in range(len(symbols)):
            s = symbols[i]
            self.__asymbols[s.addr] = i
            self.__nsymbols[s.name] = s

    def get_by_name(self, n):
        return self.__nsymbols.get(n, None)

    def get_by_addr(self, a):
        s = self.__asymbols.get(a, None)
        if s is not None:
            return self.__symbols[s]

        for s in self.__symbols:
            if a >= s.addr and a < s.addr + s.size:
                return s

        return None

    def __len__(self):
        return len(self.__symbols)

    def __iter__(self):
        return iter(self.__symbols)

    def __getitem__(self, k):
        if isinstance(k, str):
            return self.get_by_name(k)

        return self.get_by_addr(k)

    def __repr__(self):
        r = ""
        for s in self.__symbols:
            r += repr(s)+"\n"
        return r
