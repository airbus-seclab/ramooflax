#!/usr/bin/env python
#
# We are looking for "break" running under debian
#
from ramooflax import VM, Utils, CPUFamily, OSAffinity

# Some offsets for debian 2.6.32-5-486 kernel
settings = {"thread_size":8192, "comm":540, "next":240, "mm":268, "pgd":36}
os = Utils.create_os(OSAffinity.Linux26, settings)
hook = os.find_process_filter("break")

Utils.info  = True
#Utils.debug = True

#
# Main
#
vm = VM(CPUFamily.AMD, "192.168.254.254:1234")

vm.attach()
vm.stop()
vm.cpu.breakpoints.add_data_w(vm.cpu.sr.tr+4, 4, hook)

while not vm.resume():
    continue

vm.cpu.breakpoints.remove(1)
vm.cpu.set_active_cr3(os.get_process_cr3(), affinity=OSAffinity.Linux26)
print "found break process"

#
# Breakpoints handling
#

#1
vm.cpu.breakpoints.remove()
vm.cpu.breakpoints.add_insn(0x804844b)
vm.cpu.breakpoints.add_insn(0x804846b, lambda x:False)
while vm.resume():
    continue
if vm.cpu.gpr.pc != 0x804846b:
    print "failure 1"
    vm.detach()

print "done 1"

#2
vm.cpu.breakpoints.remove()
vm.cpu.breakpoints.add_insn(0x8048483)
vm.resume()
vm.singlestep()
if vm.cpu.gpr.pc != 0x8048485:
    print "failure 2"
    vm.detach()

print "done 2"

#3
vm.cpu.breakpoints.remove()
vm.cpu.breakpoints.add_hw_insn(0x804849e)
vm.resume()
vm.singlestep()
if vm.cpu.gpr.pc != 0x80484a1:
    print "failure 3"
    vm.detach()

print "done 3"

#4
vm.cpu.breakpoints.remove()
vm.cpu.breakpoints.add_insn(0x80484b1)
vm.resume()
vm.singlestep()
vm.cpu.breakpoints.add_hw_insn(0x80484d0)
vm.cpu.breakpoints.add_hw_insn(0x80484e7)
vm.resume()
vm.singlestep()
vm.singlestep()
vm.resume()
if vm.cpu.gpr.pc != 0x80484e7:
    print "failure 4"
    vm.detach()

print "done 4"

#5
vm.cpu.breakpoints.remove()
vm.cpu.breakpoints.add_insn(0x80484fa)
vm.resume()
vm.singlestep()
vm.cpu.breakpoints.add_hw_insn(0x804850a)
vm.cpu.breakpoints.add_hw_insn(0x8048523)
vm.cpu.breakpoints.add_insn(0x8048530)
vm.resume()
vm.singlestep()
vm.singlestep()
vm.resume()
vm.cpu.breakpoints.remove(2)
vm.resume()

if vm.cpu.gpr.pc != 0x8048530:
    print "failure 5"
    vm.detach()

print "done 5"

#6
vm.cpu.breakpoints.remove()
vm.cpu.breakpoints.add_insn(0x8048540)
vm.resume()
vm.cpu.breakpoints.add_data_w(vm.cpu.gpr.esp+0x1c, 4, name="stack")
vm.resume()
vm.cpu.breakpoints.remove("stack")
vm.cpu.breakpoints.add_hw_insn(0x804855b, name="insn")
vm.resume()

if vm.cpu.gpr.pc != 0x804855b:
    print "failure 6"
    vm.detach()

vm.cpu.breakpoints.remove()

print "done 6"

#
# Finished
#
vm.detach()
print "success"
