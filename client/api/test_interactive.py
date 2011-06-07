#!/usr/bin/env python

from vm import *

#
# Main
#
utils.debug = True

# Test case: get interactive

vm = VM(CPUFamily.AMD, 32, "192.168.254.254:1234")
vm.run(dict(globals(), **locals()))
