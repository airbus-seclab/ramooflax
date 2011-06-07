#!/usr/bin/env python

#
# We are looking for "argv[1]" running under win7
#
# Modus operandi:
# --------------
#
# We install a filter on cr3 writes
#
# On each write, the vmm gives us control before
# the write operation
#
# get KPRCB at [fs:0x20]
# get KTHREAD at [KPRCB+0x4]
# get EPROCESS at [KTHREAD+0x150]
# get CR3 at [EPROCESS+0x18]
# get NAME at [EPROCESS+0x16c]
# next EPROCESS at [EPROCESS+0xb8] - 0xb8
#
import sys
from vm import *

if len(sys.argv) < 2:
    print "gimme prog name"
    sys.exit(-1)

# Target process and its cr3
process_name = sys.argv[1]
process_cr3 = 0

def find_process(vm):
    kprcb    = vm.mem.read_dword(vm.cpu.sr.fs+0x20)
    kthread  = vm.mem.read_dword(kprcb+4)
    eprocess = vm.mem.read_dword(kthread+0x150)

    while eprocess != 0:
        name = vm.mem.vread(eprocess+0x16c,32)
        print "process",name
        if process_name in name:
            process_cr3  = vm.mem.read_dword(eprocess+0x18)
            print "===> process cr3",hex(process_cr3)
            return True
        elif "Idle" in name:
            return False
        eprocess = vm.mem.read_dword(eprocess+0xb8)-0xb8
    return False

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
vm.cpu.set_active_cr3(process_cr3, True, OSAffinity.Win7)
print "active cr3 installed"

vm.detach()
