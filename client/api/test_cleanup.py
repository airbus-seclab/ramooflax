#!/usr/bin/env python

from vm import *

#
# Main
#
vm = VM(CPUFamily.AMD, 32, "192.168.254.254:1234")

vm.attach()
vm.stop()
vm.cpu.del_active_cr3()
vm.detach()
