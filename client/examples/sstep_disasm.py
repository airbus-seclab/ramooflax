#!/usr/bin/env python
#
# This script auto disassemble code at each
# single step
#
# You are free to use your prefered disasm engine
# the callback provides current code location
# and bytes at that location
#
# This script uses amoco engine (https://github.com/bdcht/amoco)
#
from ramooflax import VM, Utils, CPUFamily
from amoco.arch.x86 import cpu_x86 as am

Utils.debug = True

def sstep_disasm(vm):
    code_loc = vm.cpu.code_location()
    code_bytes = vm.mem.vread(code_loc, 15)
    print "(%dbit) pc = %#x | %s" % (vm.cpu.mode,code_loc,code_bytes.encode('hex'))

    print am.disassemble(code_bytes, address=code_loc)
    return True

#
# Main
#
#peer = "192.168.254.254:1234"
peer = "172.16.131.128:1337"
vm = VM(CPUFamily.Intel, peer)

vm.attach()
vm.stop()
vm.cpu.breakpoints.filter(None, sstep_disasm)

print "\n####\n#### type: vm.singlestep()\n####\n"
vm.interact(dict(globals(), **locals()))
vm.detach()
