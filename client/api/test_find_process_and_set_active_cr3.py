#!/usr/bin/env python

#
# We are looking for "argv[1]" running under debian
#
# Modus operandi:
# --------------
#
# We install a filter on cr3 writes
#
# On each write, the vmm gives us control before
# the write operation
#
# We inspect TSS.esp0, align on THREAD_SIZE
# to get thread_info, then task_struct and mm_struct
#
# (1) vm.cpu.sr.cr3 may not be the task' one
# so do not rely on it
#
# (2) we prefer walking task list because
# nothing guarantees that each cr3 will be written
# by the kernel (ie. un-scheduled process)
#

import sys
from vm import *

if len(sys.argv) < 2:
    print "gimme prog name"
    sys.exit(-1)

# Target process and its cr3
process_name = sys.argv[1]
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
vm.cpu.set_active_cr3(process_cr3, True, OSAffinity.Linux26)
print "active cr3 installed"

vm.detach()
