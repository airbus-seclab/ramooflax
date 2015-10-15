#!/usr/bin/env python

import struct
from   ramooflax import VM, CPUFamily

#
# Main
#
vm = VM(CPUFamily.Intel, "172.16.131.128:1337")
vm.attach()
vm.stop()

sz  = vm.cpu.sr.gdtr_limit + 1
gdt = vm.mem.vread(vm.cpu.sr.gdtr_base, sz)

vm.detach()

nr  = sz/8
gdt = struct.unpack("<%dQ" % nr, gdt)
for i in xrange(len(gdt)):
    print "+0x%04x   index 0x%03x   Desc   0x%016x" % \
        (i*8,i,gdt[i])

