#!/bin/bash

#############################################
#
# Create a raw qemu bootable disk :
#
# - one partition
# - formated in e2fs
# - with grub installed on it
#
# The disk can then be converted
# to another format using qemu-img convert
#
##############################################

function log()
{
    echo $* >&2
}

# need error
function need_err()
{
    prog=$(which ${1})
    if [ "x" = "x${prog}" ];then
	log "I need ${1}"
	return 1
    fi

    echo $prog
    return 0
}

# usage
function usage()
{
    echo >&2
    echo "Usage: $0 <disk> <size> <grub dir> <kernel> [mods]" >&2
    echo "      <disk>      vdisk filename" >&2
    echo "      <size>      vdisk size (same as 'dd' tool)" >&2
    echo "      <grub dir>  grub stage files directory" >&2
    echo "      <kernel>    kernel file for grub" >&2
    echo "      [mods]      list of optional modules to load" >&2
    echo >&2 ; return 1
}

# get a loop device
function acquire_loop_dev()
{
    lo_dev=$(${LOSETUP} -f)
    if [ $? -ne 0 ]; then
	log "No more free loop device: try \"losetup -d /dev/loopX\" to release one"
	exit 1
    fi
    echo $lo_dev
}

# build a disk image of given name, size
function build_disk()
{
    disk=$1
    size=$2

    log "[+] building disk image (${size}B)"
    ${DD} if=/dev/zero of=${disk} bs=${size} count=1 2>&1 \
	| ${GREP} bytes | ${CUT} -d' ' -f1
}

function part_disk()
{
    disk=$1
    size=$2

    log "[+] create part on disk image"
    lo_disk=$(acquire_loop_dev)
    x=$(${LOSETUP} $lo_disk ${disk})
    ${SFDISK} --no-reread ${lo_disk} <<EOF >/dev/null 2>&1
;
;
;
;
EOF
    echo $lo_disk
}

# format disk at given offset
function format_disk()
{
    log "[+] formating disk"
    lo_part=$(acquire_loop_dev)
    x=$(${LOSETUP} -o ${2} $lo_part ${1})
    x=$(${MKFS} $lo_part 2>/dev/null)

    echo $lo_part
}

# mount given disk at given location
function mount_disk()
{
    part=$1
    dir=$2

    log "[+] mounting disk"
    ${MKDR} ${dir}
    ${MNT}  ${part} ${dir}
}

function umount_disk()
{
    log "[+] unmounting disk"
    ${UMNT} $1
    ${RM} -rf $1
}

function copy_grub()
{
    tdir=$1
    gdir=$2

    log "[+] copy grub"
    ${MKDR} -p ${tdir}/boot/grub
    ${CP} ${gdir}/{stage1,stage2,e2fs_stage1_5} ${tdir}/boot/grub
}

function configure_grub()
{
    tdir=$1
    shift
    kline=$*

    log "[+] configure grub"
    ${CAT} >${tdir}/boot/grub/menu.lst <<EOF
timeout 0
title kernel
root (hd0,0)
kernel $kline
EOF
}

function add_grub()
{
    tdir=$1
    echo "$2" >> ${tdir}/boot/grub/menu.lst
}

# Grub does not support number in device name
# but will add number to given disk for each
# found partition
function install_grub()
{
    tdir=$1
    disk=$2
    part=$3

    log "[+] install grub"
    grub_disk="/dev/grubdisk"
    grub_part="${grub_disk}1"
    ${LN} -s $disk $grub_disk
    ${LN} -s $part $grub_part

    #force no probe
    ${CAT} >${tdir}/boot/grub/device.map <<EOF
(hd0) $grub_disk
EOF

    ${GRUB} --batch --device-map=${tdir}/boot/grub/device.map <<EOF >/dev/null
device (hd0) $grub_disk
root (hd0,0)
setup (hd0)
EOF
    ${RM} -f $grub_disk $grub_part
}

function do_grub()
{
    if [ $# -lt 4 ]; then
	usage ${prog}
	return 1
    fi

    tdir="/tmp/$(basename ${0})_$$"
    file=${1}
    size=${2}
    gdir=${3}
    kernel=${4}
    shift 4
    mods=${*}

    bsize=$(build_disk $file $size)
    disk=$(part_disk $file $bsize)
    offset=512
    part=$(format_disk $file $offset)

    mount_disk $part $tdir
    copy_grub ${tdir} ${gdir}

    log "[+] installing files"
    ${CP} ${kernel} $mods ${tdir}/boot/

    configure_grub ${tdir} /boot/$(basename $kernel)
    for m in $mods;do
	add_grub ${tdir} "module /boot/$(basename $m)"
    done
    install_grub ${tdir} ${disk} ${part}

    umount_disk $tdir
    ${LOSETUP} -d $part $disk

    echo $offset
    return 0
}


#################################
#
#             MAIN
#
#################################

# root check
if [ $(id -u) -ne 0 ]; then
    log "Must be root: try \"sudo $*\""
    return 1
fi

# Tools
LOSETUP=$(need_err "losetup") || exit 1
DD=$(need_err "dd") || exit 1
GREP=$(need_err "grep") || exit 1
CUT=$(need_err "cut") || exit 1
SFDISK=$(need_err "sfdisk") || exit 1
MKFS=$(need_err "mkfs.ext2") || exit 1
MKDR=$(need_err "mkdir") || exit 1
MNT=$(need_err "mount") || exit 1
UMNT=$(need_err "umount") || exit 1
CP=$(need_err "cp") || exit 1
LN=$(need_err "ln") || exit 1
RM=$(need_err "rm") || exit 1
CAT=$(need_err "cat") || exit 1
TAR=$(need_err "tar") || exit 1
GRUB=$(need_err "grub") || exit 1

# params
if [ $# -lt 1 ]; then
    usage
    exit 1
fi

off=$(do_grub $*) || exit 1

log -e "\nDisk is ready !"
log " - mnt it using : \"mount -o loop,offset=${off} $1 /mnt\""
log " - run it using : \"qemu -hda $1\""
