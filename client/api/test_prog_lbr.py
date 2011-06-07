#!/usr/bin/env python

#
# We are looking for "prog" running under debian. The program
# waits for a user to hit a key and then #PF.
# We would like to get the #PF and show what was
# the last jump done by the cpu before the #PF.
# In other words, we want to know where we come from right
# before the #PF occured.
#
# This can be interesting when obfuscated programs
# jump to random box when detecting faulting condition
# to prevent debugging and exploitation
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
# Once the process found, we force the vmm
# to specificaly work on the process' cr3
#
# We install a filter on exception #PF
#
# We enable Last Branch Record cpu feature
# to keep track of last jmp
#
# Below, the script output:
# ------------------------
#
# task gvfsd-trashaem
# task mixer_applet2i
# task gvfsd-burnaem
# [ ... ]
# task gnome-terminal
# task gnome-pty-helpe
# task bash-terminal
# task prog-terminal
# ===> task cr3 0x34df1000L
# Page Fault @ 0x0
# eax     = 0x00000000
# ecx     = 0xbff20fe8
# edx     = 0x00000001
# ebx     = 0xb77d8ff4
# esp     = 0xbff20fcc
# ebp     = 0xbff20ff8
# esi     = 0x00000000
# edi     = 0x00000000
# eip     = 0x00000000
# eflags  = 0x00200207
# cs      = 0x0073
# ss      = 0x007b
# ds      = 0x007b
# es      = 0x007b
# fs      = 0x0000
# gs      = 0x0033
# 
# from_excp  = 0x00000000c1094835
# to         = 0x0000000000000000
# to_excp    = 0x00000000c1003063
# from       = 0x0000000008048431
# 
# disconnected from remote
#
# We see that eip = 0, without LBR we would have been
# unable to detect where the #PF has been triggered
#
# With the LBR, we can see that we come from 0x8048431
#

from vm import *

#utils.debug = True

# Some offsets for debian 2.6.32-5-486 kernel
com_off = 540
next_off = 240
mm_off  = 268
pgd_off = 36

# Target process and its cr3
process_name = "prog"
process_cr3 = 0

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
# Print eip on raised page fault
#
def pf_hdl(vm):
    print "Page Fault @", hex(vm.cpu.gpr.pc)
    return True

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
vm.cpu.set_active_cr3(process_cr3)
vm.cpu.filter_exception(CPUException.page_fault, pf_hdl)
vm.cpu.lbr.enable()

vm.resume()

print vm.cpu.gpr
print vm.cpu.lbr

vm.detach()
