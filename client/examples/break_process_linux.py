#!/usr/bin/env python
#
# We are looking for "break" running under debian
#
from ramooflax import VM, CPUFamily, OSFactory, OSAffinity, log, Log

# Some offsets for debian 2.6.32-5-486 kernel
#settings = {"thread_size":8192, "comm":540, "next":240, "mm":268, "pgd":36}

# Some offsets for kernel 3.4.1
settings = {"thread_size":8192, "comm":0x1cc, "next":0xc0, "mm":0xc8, "pgd":0x24}

os = OSFactory(OSAffinity.Linux26, settings)
hook = os.find_process_filter("break")

# create logging for this script
log.setup(info=(True,Log.blue), fail=(True,Log.red),
          brk=True, gdb=True, vm=True, evt=True)

#
# Main
#
vm = VM(CPUFamily.Intel, "172.16.131.128:1337")

vm.attach()
vm.stop()
vm.cpu.breakpoints.add_data_w(vm.cpu.sr.tr_base+4, 4, hook)

while not vm.resume():
    continue

vm.cpu.breakpoints.remove(1)
vm.cpu.set_active_cr3(os.get_process_cr3(), affinity=OSAffinity.Linux26)
log("info", "found break process")

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
    log("fail", "failure 1")
    vm.detach(leave=True)

log("info", "done 1")

#2
vm.cpu.breakpoints.remove()
vm.cpu.breakpoints.add_insn(0x8048483)
vm.resume()
vm.singlestep()
if vm.cpu.gpr.pc != 0x8048485:
    log("fail", "failure 2")
    vm.detach(leave=True)

log("info", "done 2")

#3
vm.cpu.breakpoints.remove()
vm.cpu.breakpoints.add_hw_insn(0x804849e)
vm.resume()
vm.singlestep()
if vm.cpu.gpr.pc != 0x80484a1:
    log("fail", "failure 3")
    vm.detach(leave=True)

log("info", "done 3")

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
    log("fail", "failure 4")
    vm.detach(leave=True)

log("info", "done 4")

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
    log("fail", "failure 5")
    vm.detach(leave=True)

log("info", "done 5")

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
    log("fail", "failure 6")
    vm.detach(leave=True)

vm.cpu.breakpoints.remove()

log("info", "done 6")

#
# Finished
#
vm.detach()
log("info", "success")
