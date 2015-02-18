#
# Copyright (C) 2015 EADS France, stephane duverger <stephane.duverger@eads.net>
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
import log

class OSAffinity:
    Unknown = 0
    Linux26 = 1
    Win7    = 2
    WinXP   = 3

def OSFactory(affinity, settings=None):
    if affinity == OSAffinity.Linux26:
        return Linux26(settings)
    elif affinity == OSAffinity.WinXP:
        return WinXP(settings)
    elif affinity == OSAffinity.Win7:
        return Win7(settings)

    log.log("error", "Unknown OS affinity")
    raise ValueError

#
# Find process under Linux 2.6 kernel
#
# We inspect TSS.esp0, aligned on THREAD_SIZE
# to get thread_info, then task_struct and mm_struct
#
# (1) vm.cpu.sr.cr3 may not be the task one
#     so we do not rely on it
#
# (2) we prefer walking task list because
#     nothing can guarantee that each cr3
#     will be scheduled by the kernel
#
class Linux26:
    #kernel info ["thread_size", "comm", "next", "mm", "pgd", "cred", "euid"]
    def __init__(self, settings):
        self.__settings = dict(settings)
        self.__msk = 0xffffffff & ~(settings["thread_size"] - 1)
        self.__task = None
        self.__pname = None
        self.__pcr3 = None
        self.__peuid = None

    def __next_task(self, vm, task):
        next = vm.mem.read_dword(task+self.__settings["next"])
        next -= self.__settings["next"]
        return next

    def __walk_process(self, vm, task):
        head = task
        while True:
            mm = vm.mem.read_dword(task+self.__settings["mm"])
            if mm != 0:
                comm = task+self.__settings["comm"]
                name = vm.mem.vread(comm, 15)
                pgd  = vm.mem.read_dword(mm+self.__settings["pgd"])
                log.log("os", "task %s" % name)
                if self.__pname in name:
                    self.__task = task
                    self.__pcr3 = pgd - 0xc0000000
                    log.log("os", "cr3 %#x" % self.__pcr3)
                    return True

            task = self.__next_task(vm, task)
            if task == head:
                return False

    def __find_process(self, vm):
        esp0 = vm.mem.read_dword(vm.cpu.sr.tr+4)
        thread_info = esp0 & self.__msk
        task = vm.mem.read_dword(thread_info)
        if task == 0:
            return False
        return self.__walk_process(vm, task)

    def __set_euid(self, vm):
        cred = vm.mem.read_dword(self.__task + self.__settings["cred"])
        vm.mem.write_dword(cred + self.__settings["euid"], self.__peuid)
        return True

    def get_process_cr3(self):
        if self.__pcr3 != None:
            return self.__pcr3
        raise ValueError

    def find_process_filter(self, name):
        self.__pname = name
        self.__pcr3 = None
        return self.__find_process

    #
    # May return False while interactive
    # if the vmm gives you control on
    # task == 0, better to use filter
    #
    def find_process(self, name, vm):
        if self.__pname != name or self.__pcr3 == None:
            self.__pname = name
            self.__pcr3 = None
            self.__peuid = None
            return self.__find_process(vm)
        return True

    def _set_euid(self, vm):
        if self.__find_process(vm):
            return self.__set_euid(vm)
        return False

    def set_euid_filter(self, name, euid):
        self.__pname = name
        self.__pcr3 = None
        self.__peuid = euid
        return self._set_euid

    def set_euid(self, name, euid, vm):
        if self.find_process(name, vm):
            if self.__peuid != euid:
                self.__peuid = euid
                return self.__set_euid(vm)
        return False

#
# Find process under Windows
#
# get KPRCB at [fs:"kprcb"]
# get KTHREAD at [KPRCB+"kthred"]
# get EPROCESS at [ETHREAD+"eprocess"]
# get CR3 at [EPROCESS+"cr3"] (as a KPROCESS for XP)
# get NAME at [EPROCESS+"name"]
# next EPROCESS at [EPROCESS+"next"] - "next"
#
class Windows:
    def __init__(self, settings):
        self.__settings = dict(settings)
        self.__pname = None
        self.__pcr3 = None

    def __find_process(self, vm):
        kprcb    = vm.mem.read_dword(vm.cpu.sr.fs+self.__settings["kprcb"])
        kthread  = vm.mem.read_dword(kprcb+self.__settings["kthread"])
        eprocess = vm.mem.read_dword(kthread+self.__settings["eprocess"])

        while eprocess != 0:
            name = vm.mem.vread(eprocess+self.__settings["name"],16)
            log.log("os", "process %s" % name[:name.index('\x00')])
            if self.__pname in name:
                self.__pcr3 = vm.mem.read_dword(eprocess+self.__settings["cr3"])
                log.log("os", "cr3 %#x" % self.__pcr3)
                return True
            elif "Idle" in name:
                return False
            eprocess  = vm.mem.read_dword(eprocess+self.__settings["next"])
            eprocess -= self.__settings["next"]
        return False

    def find_process_filter(self, name):
        self.__pname = name
        self.__pcr3 = None
        return self.__find_process

    #
    # May return False while interactive
    # if the vmm gives you control on Idle
    #
    def find_process(self, name, vm):
        if self.__pname != name or self.__pcr3 == None:
            self.__pname = name
            self.__pcr3 = None
            return self.__find_process(vm)
        return True

    def get_process_cr3(self):
        if self.__pcr3 != None:
            return self.__pcr3
        raise ValueError

class WinXP(Windows):
    def __init__(self, settings):
        Windows.__init__(self, settings)

class Win7(Windows):
    def __init__(self, settings):
        Windows.__init__(self, settings)
