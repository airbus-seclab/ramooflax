#!/usr/bin/env python
from ramooflax import VM, CPUFamily

#
# Main
#
vm = VM(CPUFamily.AMD, "192.168.254.254:1234")

vm.attach()
vm.stop()
vm.cpu.del_active_cr3()
vm.detach()
