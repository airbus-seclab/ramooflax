#!/usr/bin/env python

#
# We are looking for "stepme" running under debian
#
# Modus operandi:
# --------------
#
# We install a hardware data w breakpoint on TSS.esp0
# location. On modern operating systems, the kernel
# writes TSS.esp0 when scheduling a process to ensure
# the cpu will use the right kernel stack on idt events
#
# On each write, the vmm gives us control "AFTER"
# the write operation
#
# We inspect TSS.esp0, align on THREAD_SIZE
# to get thread_info, then task_struct and mm_struct
#
# (1) vm.cpu.sr.cr3 may not be the task' one
# so do not rely on it
#
# (2) we prefer walking task list because
# of scheduling impact due to vmexits
#

from vm import *

#utils.debug = True

# Some offsets for debian 2.6.32-5-486 kernel
com_off  = 540
next_off = 240
mm_off   = 268
pgd_off  = 36

# Target process and its cr3
process_cr3  = 0
process_name = "stepme"
process_brk  = 0x8048425

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
            if process_name in name:
                process_cr3 = pgd - 0xc0000000
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

vm.cpu.breakpoints.add_data_w(vm.cpu.sr.tr+4, 4, find_process, "esp0")

while not vm.resume():
    continue

vm.cpu.breakpoints.remove("esp0")

print "found %s cr3 0x%.8x" % (process_name,process_cr3)

vm.cpu.set_active_cr3(process_cr3)
#vm.cpu.breakpoints.add_hw_insn(process_brk, name="after_read")
vm.cpu.breakpoints.add_insn(process_brk, name="after_read")
print "hit keyboard on VM"
vm.resume()
vm.cpu.breakpoints.remove("after_read")
vm.interact(dict(globals(), **locals()))

#vm.detach()
