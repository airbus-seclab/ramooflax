#!/usr/bin/env python

import string, os, sys

if len(sys.argv) != 2:
    print "usage:",sys.argv[0],"<file.bin>"
    sys.exit(0)

#read symbols
symbols = []
addr = []
kernel = sys.argv[1]
cmdline="nm -f sysv "+kernel+" | grep FUNC | cut -d '|' -f1,2,5"
fd = os.popen(cmdline)
line = fd.readline()
while line != "":
    name,addr,size = line[:-1].split("|")
    name = name.strip()
    addr = int(addr.strip(),16)
    size = size.strip()
    if size == "":
	size = 0
    else:
	size = int(size,16)

    symbols.append( [addr, size, name] )
    line = fd.readline()
fd.close()

#sentinel
symbols.append( [ 0xffffffff, 0xffffffff, 'undefined' ] )
symbols.sort()

#recompute incorrect symbol size
for i in range(len(symbols)):
    if symbols[i][1] == 0:
	symbols[i][1] = symbols[i+1][0] - symbols[i][0]

#read trace
fd = sys.stdin
line = fd.readline()
print "\n == VMM Trace ==\n"
while line != "":
    addr = line[:-1]
    if addr == "":
        line = fd.readline()
        continue

    addr = int(addr,16)
    for i in range(len(symbols)):
        if addr >= symbols[i][0] and addr < symbols[i][0]+symbols[i][1]:
            print "[RIP 0x%.16x FCT 0x%.16x] %s"%(addr,symbols[i][0],symbols[i][2])
            sys.stdout.flush()
            break

    line = fd.readline()
fd.close()
