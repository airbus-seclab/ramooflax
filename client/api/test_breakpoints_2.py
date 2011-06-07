#!/usr/bin/env python

#
# To be tested with LoopOS
#
#0020009c <setup>:
#  20009c:       55                      push   %ebp
#  20009d:       89 e5                   mov    %esp,%ebp
#  20009f:       83 ec 18                sub    $0x18,%esp
#  2000a2:       0f 20 d8                mov    %cr3,%eax
#  2000a5:       89 45 fc                mov    %eax,-0x4(%ebp)
#  2000a8:       c7 04 24 aa 09 20 00    movl   $0x2009aa,(%esp)
#  2000af:       e8 48 01 00 00          call   2001fc <printf>
#  2000b4:       8b 45 fc                mov    -0x4(%ebp),%eax
#  2000b7:       0f 22 d8                mov    %eax,%cr3
#  2000ba:       eb e6                   jmp    2000a2 <setup+0x6>
#

# Test case: breakpoint on emulated insn and single-step !

from vm import *

#utils.debug = True

def print_excp(vm):
    excp = vm.cpu.last_reason - StopReason.excp_de
    print "#%d raised" % (excp)
    return True

vm = VM(CPUFamily.AMD, 32, "192.168.254.254:1234")

vm.attach()
vm.stop()
vm.cpu.lbr.enable()

vm.cpu.filter_exception(CPUException.double_fault, print_excp)
vm.cpu.filter_exception(CPUException.general_protection, print_excp)
vm.cpu.filter_exception(CPUException.debug, print_excp)
vm.cpu.filter_exception(CPUException.breakpoint, print_excp)

vm.cpu.breakpoints.add_insn(0x2000b7)
vm.resume()
vm.singlestep()

if vm.cpu.gpr.pc == 0x2000ba:
    print "success"
else:
    print "failure"

vm.detach()
