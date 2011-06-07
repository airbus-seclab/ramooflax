#!/usr/bin/env python

import sys
from vm import *

# Target process and its cr3
process_name = "syscall"
process_cr3 = 0

#utils.debug = True

# Some offsets for debian 2.6.32-5-486 kernel
com_off = 540
next_off = 240
mm_off  = 268
pgd_off = 36

def next_task(vm, task):
    next = vm.mem.read_dword(task+next_off)
    next -= next_off
    return next

def walk_process(vm, task):
    global process_cr3
    head = task
    while True:
        mm = vm.mem.read_dword(task+mm_off)
        if mm != 0:
            comm = task+com_off
            name = vm.mem.vread(comm, 15)
            pgd  = vm.mem.read_dword(mm+pgd_off)
            print "task",name
            if process_name in name:
                process_cr3 = pgd - 0xc0000000
                print "===> task cr3",hex(process_cr3)
                return True

        task = next_task(vm, task)
        if task == head:
            return False

#
# A filter is a simple function taking
# vm as a parameter and returning True if
# it wants the vm to stop (interactive)
#
def find_process(vm):
    esp0 = vm.mem.read_dword(vm.cpu.sr.tr+4)
    thread_info = esp0 & 0xffffe000
    task = vm.mem.read_dword(thread_info)
    if task == 0:
        return False
    return walk_process(vm, task)

#
# Main
#
vm = VM(CPUFamily.AMD, 32, "192.168.254.254:1234")

vm.attach()
vm.stop()
vm.cpu.filter_write_cr(3, find_process)

while not vm.resume():
    continue

vm.cpu.release_write_cr(3)

print "found %s cr3 0x%.8x" % (process_name,process_cr3)

vm.cpu.set_active_cr3(process_cr3, True, OSAffinity.Linux26)

vm.cpu.breakpoints.add_hw_insn(0x8048452, name="after_read")
vm.cpu.breakpoints.add_hw_insn(0xb7fe2423, name="before_sysenter")

print "hit a key on VM ..."

vm.resume()
vm.interact(dict(globals(), **locals()))
