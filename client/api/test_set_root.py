#!/usr/bin/env python

import sys
from vm import *

if len(sys.argv) < 3:
    print "usage: <prog name> <desired uid>"
    sys.exit(-1)

# Target process
process_name = sys.argv[1]
uid = int(sys.argv[2])

#utils.debug = True

# Some offsets for debian 2.6.32-5-486 kernel
com_off  = 540
next_off = 240
cred_off = 520
euid_off = 20

def next_task(vm, task):
    next = vm.mem.read_dword(task+next_off)
    next -= next_off
    return next

def walk_process(vm, task):
    global uid
    head = task
    while True:
        comm = task+com_off
        name = vm.mem.vread(comm, 15)
        print "task",name
        if process_name in name:
            cred = vm.mem.read_dword(task+cred_off)
            vm.mem.write_dword(cred+euid_off, uid)
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

print "success"
vm.detach()
