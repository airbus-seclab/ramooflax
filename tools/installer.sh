#!/bin/bash

VMNAME="Debian 7 64-bit"

VMPATH="/media/psf/Home/Vmware/${VMNAME}.vmwarevm"
VMDISK="${VMPATH}/${VMNAME}.vmdk"
VMMNT="${VMPATH}/mnt"

vmware-mount ${VMDISK} 1 ${VMMNT} || exit 1
cp $1 "${VMMNT}/boot/$(basename $1)" || exit 1
vmware-mount -X 2>/dev/null
exit 0
