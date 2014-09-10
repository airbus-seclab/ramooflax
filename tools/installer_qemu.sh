#!/bin/bash

VMNAME="ramooflax"
VMPATH="/home/stf/Work/vm/qemu/${VMNAME}"
VMDISK="${VMPATH}/${VMNAME}.img"
VMMNT="${VMPATH}/mnt"

mount -o loop,offset=512 ${VMDISK} ${VMMNT} || exit 1
cp $1 "${VMMNT}/boot/$(basename $1)" || exit 1
sleep .5
umount ${VMMNT}
exit 0
