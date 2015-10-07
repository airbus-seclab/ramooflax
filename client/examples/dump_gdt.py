#!/usr/bin/env python
from ramooflax import VM, CPUFamily
import struct

#
# Main
#
vm = VM(CPUFamily.Intel, "172.16.131.128:1337")
vm.attach()
vm.stop()

#TODO: provide access to gdtr.limit
nr = 40

fmt = "<%dQ" % nr
gdt = struct.unpack(fmt, vm.mem.vread(vm.cpu.sr.gdtr, nr*8))

for i in xrange(len(gdt)):
    print "+0x%04x   index 0x%03x   Desc   0x%016x" % \
        (i*8,i,gdt[i])

vm.detach()
