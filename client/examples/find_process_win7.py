#!/usr/bin/env python
#
# We are looking for "argv[1]" running under windows 7
#
# We install a filter on cr3 writes
# On each write, the vmm gives us control
# before the write operation
#
from ramooflax import VM, CPUFamily, Utils, OSAffinity
import sys

if len(sys.argv) < 2:
    print "gimme prog name"
    sys.exit(-1)

# Target process
process_name = sys.argv[1]

# Some offsets for Windows 7 Premium FR 32 bits
settings = {"kprcb":0x20, "kthread":4,
            "eprocess":0x150, "name":0x16c,
            "cr3":0x18, "next":0xb8}

os = Utils.create_os(OSAffinity.Win7, settings)
hook = os.find_process_filter(process_name)

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
print "success: %#x" % (os.get_process_cr3())
vm.detach()
