#!/bin/bash

VMNAME="Debian 7 64-bit"
VMPATH="/media/psf/Home/Vmware/${VMNAME}.vmwarevm"

vmware-mount "${VMPATH}/${VMNAME}.vmdk" 1 "${VMPATH}/mnt"
cp $1 "${VMPATH}/mnt/boot/$(basename $1)"
vmware-mount -X 2>/dev/null

exit 0
