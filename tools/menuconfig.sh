#!/bin/bash

conf=$1
if [ "x${conf}" == "x" ]; then
    echo "missing config filename"
    exit 1
fi

tools_dir="./tools"
defconfin="${tools_dir}/default_config"
dialog_bin="$(which dialog)"
mktemp_bin="$(which mktemp)"

answer="$(${mktemp_bin} --suffix $$)"
exec {answer_fd}>${answer}

dialog="${dialog_bin} --title Ramooflax --output-fd ${answer_fd}"

function reading()
{
    confin="${conf}"
    if [ ! -f $confin ];then
	confin=$defconfin
    fi

    source $confin
}

function building()
{
    while /bin/true;
    do
	$dialog --menu "" 10 50 10 \
	    "cpu" "select manufacturer" \
	    "control" "select control device" \
	    "debug" "select debug output device" \
	    "proxy" "select various proxy modes"

	if [ $? -ne 0 ];then
	    break
	fi

	ans=$(<$answer) ; :>$answer
	if [ $ans == "cpu" ];then
	    build_proc
	elif [ $ans == "control" ];then
	    build_ctrl
	elif [ $ans == "debug" ];then
	    build_debug
	elif [ $ans == "proxy" ];then
	    build_proxy
	fi
    done
}

function build_proc()
{
    if [ $CONFIG_ARCH == "amd" ];then
	amd_sts="on"
	intel_sts="off"
    elif [ $CONFIG_ARCH == "intel" ];then
	intel_sts="on"
	amd_sts="off"
    fi

    $dialog --radiolist \
	"Choose the manufacturer on which the vmm will run" 10 50 10 \
	"intel" "" $intel_sts \
	"amd" "" $amd_sts

    if [ $? -eq 0 ];then
	$CONFIG_ARCH=$(<$answer) ; :>$answer
    fi
}

function build_ctrl()
{
    if [ $CONFIG_CTRL == "ehci" ];then
	ehci_sts="on"
	uart_sts="off"
    elif [ $CONFIG_CTRL == "uart" ];then
	ehci_sts="off"
	uart_sts="on"
    fi

    $dialog --radiolist \
	"Choose the device used to remote control the vmm" 10 50 10 \
	"ehci" "EHCI Debug Port" $ehci_sts \
	"uart" "Serial Port" $uart_sts

    if [ $? -eq 0 ];then
	$CONFIG_CTRL=$(<$answer) ; :>$answer
    fi
}

function build_debug()
{
    if [ $CONFIG_PRINT == "ehci" ];then
	ehci_sts="on"
	uart_sts="off"
    elif [ $CONFIG_PRINT == "uart" ];then
	ehci_sts="off"
	uart_sts="on"
    fi

    $dialog --radiolist \
	"Choose the device used to output debug messages" 10 50 10 \
	"ehci" "EHCI Debug Port" $ehci_sts \
	"uart" "Serial Port" $uart_sts

    if [ $? -eq 0 ];then
	$CONFIG_PRINT=$(<$answer) ; :>$answer
    fi
}

function build_proxy()
{
    $dialog --separate-output --checklist \
	"Enable/Disable proxy modes" 20 70 10 \
	"CONFIG_UART_PROXY" \
	"vm operations on uart1 are proxyfied" $CONFIG_UART_PROXY \
	"CONFIG_MSR_PROXY" \
	"vm operations on msrs are proxyfied" $CONFIG_MSR_PROXY

    if [ $? -eq 0 ];then
	ans=$(<$answer) ; :>$answer

	CONFIG_UART_PROXY="off"
	CONFIG_MSR_PROXY="off"

	for v in $ans;do
	    eval $v="on"
	done
    fi
}

function installing()
{
    $dialog --inputbox "Enter USB key mount point" 10 50 $CONFIG_INST_DIR
    if [ $? -eq 0 ];then
	CONFIG_INST_DIR=$(<$answer) ; :>$answer
    fi
}

function commit()
{
    confout="${conf}" ; :>$confout

    echo "CONFIG_ARCH=$CONFIG_ARCH" >> $confout
    echo "CONFIG_CTRL=$CONFIG_CTRL" >> $confout
    echo "CONFIG_PRINT=$CONFIG_PRINT" >> $confout
    echo "CONFIG_UART_PROXY=$CONFIG_UART_PROXY" >> $confout
    echo "CONFIG_MSR_PROXY=$CONFIG_MSR_PROXY" >> $confout
    echo "CONFIG_INST_DIR=$CONFIG_INST_DIR" >> $confout
}

#
# Main
#
reading

while /bin/true;
do
    $dialog --menu "" 10 50 10 \
	"build" "change build settings" \
	"install" "change install directory" \
	"quit" "save configuration and quit"

    if [ $? -ne 0 ];then
	break
    fi

    ans=$(<$answer) ; :>$answer
    if [ $ans == "build" ];then
	building
    elif [ $ans == "install" ];then
	installing
    elif [ $ans == "quit" ];then
	commit
	break
    fi
done

rm -f $answer