#!/usr/bin/env python
#
# We are looking for "prog" running under debian.
#
# The program waits for a user to hit a key and then #PF.
# We would like to get the #PF and show what was
# the last jump done by the cpu before the #PF.
# In other words, we want to know where we come from right
# before the #PF occured.
#
# This can be interesting when obfuscated programs
# jump to random box when detecting faulting condition
# to prevent debugging and exploitation
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
# Page Fault @ 0x0
# eax     = 0x00000000
# ecx     = 0x00000000
# edx     = 0x00000000
# ebx     = 0x00000000
# esp     = 0x00000000
# ebp     = 0x00000000
# esi     = 0x00000000
# edi     = 0x00000000
# eip     = 0x00000000
# eflags  = 0x00010246
# cs      = 0x0073
# ss      = 0x007b
# ds      = 0x007b
# es      = 0x007b
# fs      = 0x0000
# gs      = 0x0033
#
# from_excp  = 0x00000000c103cc3e
# to         = 0x0000000000000000
# to_excp    = 0x00000000c103d8bc
# from       = 0x00000000080483d7
#
# We see that eip = 0, without LBR we can't detect
# where the #PF has been triggered
# With the LBR, we can see that we come from "from"
#
from ramooflax import VM, CPUFamily, OSFactory, OSAffinity, CPUException, log

# create logging for this script
log.setup(info=True, fail=True)

# Some offsets for debian 2.6.32-5-486 kernel
settings = {"thread_size":8192, "comm":540, "next":240, "mm":268, "pgd":36}
os = OSFactory(OSAffinity.Linux26, settings)
hook = os.find_process_filter("prog")

#
# Print eip on raised page fault
#
def pf_hook(vm):
    log("info", "Page Fault @ %#x" % vm.cpu.gpr.pc)
    return True

#
# Main
#
vm = VM(CPUFamily.AMD, "192.168.254.254:1234")

vm.attach()
vm.stop()
vm.cpu.filter_write_cr(3, hook)

while not vm.resume():
    continue

vm.cpu.release_write_cr(3)
vm.cpu.set_active_cr3(os.get_process_cr3(), True, OSAffinity.Linux26)

vm.cpu.filter_exception(CPUException.page_fault, pf_hook)
vm.cpu.lbr.enable()

vm.resume()

log("info", vm.cpu.gpr)
log("info", vm.cpu.lbr)

vm.detach()
