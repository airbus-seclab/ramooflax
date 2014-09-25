#!/usr/bin/env python
#
# Enter interactive 'shell' mode
#
from ramooflax import VM, Utils, CPUFamily, OSAffinity

Utils.debug = True

#vm = VM(CPUFamily.AMD, "192.168.254.254:1234")
vm = VM(CPUFamily.Intel, "172.16.131.128:1337")
vm.run(dict(globals(), **locals()))
