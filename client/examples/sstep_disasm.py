#!/usr/bin/env python
#
# This script auto disassemble code at each
# single step
#
# You are free to use your prefered disasm engine
# the callback provides current code location
# and bytes at that location
#
# This script contains (commented) an example
# usage of Distorm3 (unstable)
#
from ramooflax import VM, Utils, CPUFamily
#from distorm3 import Decode, Decode16Bits, Decode32Bits, Decode64Bits

def sstep_disasm(vm):
    code_loc = vm.cpu.code_location()
    code_bytes = vm.mem.vread(code_loc, 15)
    print "pc = %#x | %s" % (code_loc, code_bytes.encode('hex'))

    # if vm.cpu.mode == 16:
    #     mode = Decode16Bits
    # elif vm.cpu.mode == 32:
    #     mode = Decode32Bits
    # else:
    #     mode = Decode64Bits
    #print Decode(code_loc, code_bytes, mode)[0][2]

    return True

#
# Main
#
vm = VM(CPUFamily.Intel, "192.168.254.254:1234")

vm.attach()
vm.stop()
vm.cpu.breakpoints.filter(None, sstep_disasm)

print "\n####\n#### type: vm.singlestep()\n####\n"
vm.interact(dict(globals(), **locals()))
vm.detach()
