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
import struct

from ramooflax.core import CPUMode, log

class PgMsk(object):
    present = 1<<0
    write   = 1<<1
    user    = 1<<2
    addr    = ~((1<<12)-1)
    large   = 1<<7
    laddr   = ~((1<<22)-1)

class Page(object):
    def __init__(self, vaddr, paddr, size, w, u):
        self.vaddr = vaddr
        self.paddr = paddr
        self.size  = size
        self.__u = u
        self.__w = w

        if size == (4<<10):
            self.ssize = "4KB"
        else:
            self.ssize = "4MB"

        if w:
            self.right = "rw"
        else:
            self.right = "ro"

        if u:
            self.mode = "user"
        else:
            self.mode = "krnl"

    def user(self):
        return self.__u != 0

    def rw(self):
        return self.__w != 0

    def __str__(self):
        return "vaddr 0x%.8x paddr 0x%.8x %s %s (%s)" % \
            (self.vaddr, self.paddr, self.right, self.mode, self.ssize)

class PteBase(object):
    def __init__(self, val):
        self.raw = val
        self.p = val & PgMsk.present
        self.w = val & PgMsk.write
        self.u = val & PgMsk.user
        self.addr = val & PgMsk.addr

        log("paging", str(self))

    def __str__(self):
        return "p %d w %d u %d addr 0x%x" % (self.p,self.w,self.u,self.addr)

class Pte(PteBase):
    def __init__(self, vaddr, val):
        PteBase.__init__(self, val)
        if self.p:
            self.page = Page(vaddr, self.addr, 4<<10, self.w, self.u)

class PageTable(object):
    def __init__(self, vm, vaddr, paddr):
        self.addr = paddr

        log("paging", "PageTable reading physical page @ 0x%x" % paddr)

        self.raw = struct.unpack("<1024L", vm.mem.pread(paddr, 4<<10))
        self.pte = []

        for i in xrange(1024):
            self.pte.append(Pte(vaddr, self.raw[i]))
            vaddr += 4<<10

    def __getitem__(self, i):
        if not self.pte[i].p:
            return None
        return self.pte[i].page

    def dump(self):
        for p in self:
            if p is not None:
                print p

class Pde(PteBase):
    def __init__(self, vm, vaddr, val):
        PteBase.__init__(self, val)
        self.large = val & PgMsk.large
        if self.large:
            self.addr = val & PgMsk.laddr
            if self.p:
                self.page = Page(vaddr, self.addr, 4<<20, self.w, self.u)
        elif self.p:
            self.ptb = PageTable(vm, vaddr, self.addr)

class PageDirectory(object):
    def __init__(self, vm, cr3):
        self.addr = cr3 & PgMsk.addr

        log("paging", "PageDirectory reading physical page @ 0x%x" % self.addr)

        self.raw = struct.unpack("<1024L", vm.mem.pread(self.addr, 4<<10))
        self.pde = []

        vaddr = 0
        for i in xrange(1024):
            self.pde.append(Pde(vm, vaddr, self.raw[i]))
            vaddr += 4<<20

    def __getitem__(self, i):
        if not self.pde[i].p:
            return None
        if self.pde[i].large:
            return self.pde[i].page
        return self.pde[i].ptb

    def dump(self):
        for p in self:
            if p is None:
                continue
            if isinstance(p, Page):
                print p
            else:
                p.dump()

class MemRange(object):
    def __init__(self, pg):
        self.start = pg.vaddr
        self.end = pg.vaddr+pg.size
        self.rw = pg.right
        self.md = pg.mode

    def covers(self, v):
        return self.start <= v.end and self.rw == v.rw and self.md == v.md

    def __str__(self):
        return "0x%.8x - 0x%.8x (%s,%s)\n" % (self.start,self.end,self.rw,self.md)

class Mapping(object):
    def __init__(self, pgd):
        self._s = []
        self._e = []
        self._map = []

        for p in pgd:
            if p is not None:
                if isinstance(p, Page):
                    self.add(p)
                else:
                    for pg in p:
                        if pg is not None:
                            self.add(pg)

    def add(self, pg):
        c = MemRange(pg)
        j = len(self._map)
        if j == 0:
            self._map.append(c)
            return

        p = self._map[j-1]
        if c.covers(p):
            p.end = c.end
        else:
            self._map.append(c)

    def __getitem__(self, i):
        return self._map[i]

    def __len__(self):
        return len(self._map)

class AddrSpace(object):
    def __init__(self, vm, cr3):
        if vm.cpu.mode.pg != CPUMode.pg32:
            log("error", "paging mode not supported: %s" % vm.cpu.mode)
            raise ValueError

        self.pgd = PageDirectory(vm, cr3)
        self.map = Mapping(self.pgd)

    def is_mapped(self, vaddr):
        ptb = self.pgd[vaddr>>22]
        if ptb is None:
            return False
        if isinstance(ptb, Page):
            return True
        pg =  ptb[(vaddr>>12)&0x3ff]
        return pg is not None

    def page(self, vaddr):
        ptb = self.pgd[vaddr>>22]
        if ptb is None:
            return None
        if isinstance(ptb, Page):
            return ptb
        return ptb[(vaddr>>12)&0x3ff]

    def __str__(self):
        s = "Address space has %d entries\n" % len(self.map)
        for m in self.map:
            s += str(m)
        return s

    def dump(self):
        self.pgd.dump()

    # all page tables
    def iter_pagetables(self):
        for p in self.pgd:
            if isinstance(p,PageTable):
                yield p

    # all pages, with constraints
    # - user = None, True, False
    # - rw   = None, True, False
    # - addr = None, physical address covered by a page
    # - size = None, 4<<12, 4<<20
    def iter_pages(self, addr=None, user=None, rw=None, size=None):
        def __valid(p):
            if p is None:
                return False
            if addr is not None and not (p.paddr <= addr < p.paddr + p.size):
                return False
            if user is not None and user != p.user():
                return False
            if rw is not None and rw != p.rw():
                return False
            if size is not None and size != p.size:
                return False
            return True

        for p in self.pgd:
            if isinstance(p,PageTable):
                for pg in p:
                    if __valid(pg):
                        yield pg
            elif __valid(p):
                yield p

    # search with criterion ie. user=(None,True,False)
    def search_paddr(self, paddr, user=None, rw=None):
        return list(self.iter_pages(paddr,user,rw))
