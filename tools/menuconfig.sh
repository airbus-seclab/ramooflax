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
	    "devices" "select devices" \
	    "options" "select various options"

	if [ $? -ne 0 ];then
	    break
	fi

	ans=$(<$answer) ; :>$answer
	if [ $ans == "cpu" ];then
	    build_cpu
	elif [ $ans == "devices" ];then
	    build_dev
	elif [ $ans == "options" ];then
	    build_options
	fi
    done
}

function build_cpu()
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
	CONFIG_ARCH=$(<$answer) ; :>$answer
    fi
}

function build_dev()
{
    while /bin/true;
    do
	$dialog --menu "" 10 50 10 \
	    "control" "select control device" \
	    "debug" "select debug output device"

	if [ $? -ne 0 ];then
	    break
	fi

	ans=$(<$answer) ; :>$answer
	if [ $ans == "control" ];then
	    build_ctrl
	elif [ $ans == "debug" ];then
	    build_debug
	fi
    done
}

function build_ctrl()
{
    if [ $CONFIG_CTRL == "ehci" ];then
	ehci_sts="on"
	uart_sts="off"
	none_sts="off"
    elif [ $CONFIG_CTRL == "uart" ];then
	ehci_sts="off"
	uart_sts="on"
	none_sts="off"
    elif [ $CONFIG_CTRL == "none" ];then
	ehci_sts="off"
	uart_sts="off"
	none_sts="on"
    fi

    $dialog --radiolist \
	"Choose the device used to remote control the vmm" 10 55 10\
	"ehci" "EHCI Debug Port" $ehci_sts \
	"uart" "Serial Port" $uart_sts \
	"none" "No control device" $none_sts

    if [ $? -eq 0 ];then
	CONFIG_CTRL=$(<$answer) ; :>$answer
    fi
}

function build_debug()
{
    if [ $CONFIG_PRINT == "ehci" ];then
	ehci_sts="on"
	uart_sts="off"
	none_sts="off"
    elif [ $CONFIG_PRINT == "uart" ];then
	ehci_sts="off"
	uart_sts="on"
	none_sts="off"
    elif [ $CONFIG_PRINT == "none" ];then
	ehci_sts="off"
	uart_sts="off"
	none_sts="on"
    fi

    $dialog --radiolist \
	"Choose the device used to output debug messages" 10 55 10 \
	"ehci" "EHCI Debug Port" $ehci_sts \
	"uart" "Serial Port" $uart_sts \
	"none" "No debug device" $none_sts

    if [ $? -eq 0 ];then
	CONFIG_PRINT=$(<$answer) ; :>$answer
    fi
}

function build_options()
{
    $dialog --separate-output --checklist \
	"Enable/Disable various options" 20 70 10 \
	"CONFIG_MSR_PROXY" \
	"vm operations on msrs are proxyfied" $CONFIG_MSR_PROXY \
	"CONFIG_EHCI_FULL" \
	"force full EHCI controller init" $CONFIG_EHCI_FULL \
	"CONFIG_EHCI_2ND" \
	"use 2nd EHCI controller if available" $CONFIG_EHCI_2ND

    if [ $? -eq 0 ];then
	ans=$(<$answer) ; :>$answer

	CONFIG_MSR_PROXY="off"
	CONFIG_EHCI_FULL="off"
	CONFIG_EHCI_2ND="off"

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

function debuging()
{
    $dialog --separate-output --checklist \
	"Enable/disable various debug info" 20 70 10 \
	"CONFIG_DEBUG_ACTIVE" "global vmm debugging" $CONFIG_DEBUG_ACTIVE \
	"CONFIG_VMEXIT_TRACE" "show vmexit count on each debug line" $CONFIG_VMEXIT_TRACE \
	"CONFIG_VMM_DBG" "" $CONFIG_VMM_DBG \
	"CONFIG_VM_ACCESS_DBG" "" $CONFIG_VM_ACCESS_DBG \
	"CONFIG_MP_DBG" "" $CONFIG_MP_DBG \
	"CONFIG_PG_DBG" "" $CONFIG_PG_DBG \
	"CONFIG_PG_W_DBG" "" $CONFIG_PG_W_DBG \
	"CONFIG_SMAP_DBG" "" $CONFIG_SMAP_DBG \
	"CONFIG_PMEM_DBG" "" $CONFIG_PMEM_DBG \
	"CONFIG_VMEM_DBG" "" $CONFIG_VMEM_DBG \
	"CONFIG_CPU_DBG" "" $CONFIG_CPU_DBG \
	"CONFIG_EHCI_DBG" "" $CONFIG_EHCI_DBG \
	"CONFIG_UART_DBG" "" $CONFIG_UART_DBG \
	"CONFIG_EMU_DBG" "" $CONFIG_EMU_DBG \
	"CONFIG_DIS_DBG" "" $CONFIG_DIS_DBG \
	"CONFIG_EMU_INSN_DBG" "" $CONFIG_EMU_INSN_DBG \
	"CONFIG_DIS_INSN_DBG" "" $CONFIG_DIS_INSN_DBG \
	"CONFIG_PVL_DBG" "" $CONFIG_PVL_DBG \
	"CONFIG_INSN_DBG" "" $CONFIG_INSN_DBG \
	"CONFIG_CPUID_DBG" "" $CONFIG_CPUID_DBG \
	"CONFIG_CR_DBG" "" $CONFIG_CR_DBG \
	"CONFIG_DR_DBG" "" $CONFIG_DR_DBG \
	"CONFIG_MSR_DBG" "" $CONFIG_MSR_DBG \
	"CONFIG_IRQ_DBG" "" $CONFIG_IRQ_DBG \
	"CONFIG_INT_DBG" "" $CONFIG_INT_DBG \
	"CONFIG_IO_DBG" "" $CONFIG_IO_DBG \
	"CONFIG_EXCP_DBG" "" $CONFIG_EXCP_DBG \
	"CONFIG_GP_DBG" "" $CONFIG_GP_DBG \
	"CONFIG_BP_DBG" "" $CONFIG_BP_DBG \
	"CONFIG_DB_DBG" "" $CONFIG_DB_DBG \
	"CONFIG_PF_DBG" "" $CONFIG_PF_DBG \
	"CONFIG_PF_VERBOSE_DBG" "" $CONFIG_PF_VERBOSE_DBG \
	"CONFIG_SMI_DBG" "" $CONFIG_SMI_DBG \
	"CONFIG_CTRL_DBG" "" $CONFIG_CTRL_DBG \
	"CONFIG_GDB_DBG" "" $CONFIG_GDB_DBG \
	"CONFIG_GDB_PKT_DBG" "" $CONFIG_GDB_PKT_DBG \
	"CONFIG_GDB_CMD_DBG" "" $CONFIG_GDB_CMD_DBG \
	"CONFIG_GDB_PARSE_DBG" "" $CONFIG_GDB_PARSE_DBG \
	"CONFIG_DEV_DBG" "" $CONFIG_DEV_DBG \
	"CONFIG_DEV_PIC_DBG" "" $CONFIG_DEV_PIC_DBG \
	"CONFIG_DEV_UART_DBG" "" $CONFIG_DEV_UART_DBG \
	"CONFIG_DEV_IO_DBG" "" $CONFIG_DEV_IO_DBG \
	"CONFIG_DEV_PS2_DBG" "" $CONFIG_DEV_PS2_DBG \
	"CONFIG_DEV_KBD_DBG" "" $CONFIG_DEV_KBD_DBG \
	"CONFIG_SVM_DBG" "" $CONFIG_SVM_DBG \
	"CONFIG_SVM_CPU_DBG" "" $CONFIG_SVM_CPU_DBG \
	"CONFIG_SVM_IDT_DBG" "" $CONFIG_SVM_IDT_DBG \
	"CONFIG_SVM_EXCP_DBG" "" $CONFIG_SVM_EXCP_DBG \
	"CONFIG_SVM_EXCP_GP_DBG" "" $CONFIG_SVM_EXCP_GP_DBG \
	"CONFIG_SVM_EXCP_PF_DBG" "" $CONFIG_SVM_EXCP_PF_DBG \
	"CONFIG_SVM_NPF_DBG" "" $CONFIG_SVM_NPF_DBG \
	"CONFIG_SVM_INT_DBG" "" $CONFIG_SVM_INT_DBG \
	"CONFIG_SVM_IO_DBG" "" $CONFIG_SVM_IO_DBG \
	"CONFIG_SVM_IRQ_DBG" "" $CONFIG_SVM_IRQ_DBG \
	"CONFIG_SVM_CPUID_DBG" "" $CONFIG_SVM_CPUID_DBG \
	"CONFIG_SVM_MSR_DBG" "" $CONFIG_SVM_MSR_DBG \
	"CONFIG_SVM_CR_DBG" "" $CONFIG_SVM_CR_DBG \
	"CONFIG_VMX_DBG" "" $CONFIG_VMX_DBG \
	"CONFIG_VMX_CPU_DBG" "" $CONFIG_VMX_CPU_DBG \
	"CONFIG_VMX_IDT_DBG" "" $CONFIG_VMX_IDT_DBG \
	"CONFIG_VMX_EPT_DBG" "" $CONFIG_VMX_EPT_DBG \
	"CONFIG_VMX_EXCP_DBG" "" $CONFIG_VMX_EXCP_DBG \
	"CONFIG_VMX_EXCP_PF_DBG" "" $CONFIG_VMX_EXCP_PF_DBG \
	"CONFIG_VMX_EXCP_GP_DBG" "" $CONFIG_VMX_EXCP_GP_DBG \
	"CONFIG_VMX_INT_DBG" "" $CONFIG_VMX_INT_DBG \
	"CONFIG_VMX_IO_DBG" "" $CONFIG_VMX_IO_DBG \
	"CONFIG_VMX_IRQ_DBG" "" $CONFIG_VMX_IRQ_DBG \
	"CONFIG_VMX_CPUID_DBG" "" $CONFIG_VMX_CPUID_DBG \
	"CONFIG_VMX_MSR_DBG" "" $CONFIG_VMX_MSR_DBG \
	"CONFIG_VMX_CR_DBG" "" $CONFIG_VMX_CR_DBG

    if [ $? -eq 0 ];then
	ans=$(<$answer) ; :>$answer

	CONFIG_DEBUG_ACTIVE="off"
	CONFIG_VMEXIT_TRACE="off"

	CONFIG_VMM_DBG="off"
	CONFIG_VM_ACCESS_DBG="off"
	CONFIG_MP_DBG="off"
	CONFIG_PG_DBG="off"
	CONFIG_PG_W_DBG="off"
	CONFIG_SMAP_DBG="off"
	CONFIG_PMEM_DBG="off"
	CONFIG_VMEM_DBG="off"
	CONFIG_CPU_DBG="off"
	CONFIG_EHCI_DBG="off"
	CONFIG_UART_DBG="off"
	CONFIG_EMU_DBG="off"
	CONFIG_DIS_DBG="off"
	CONFIG_EMU_INSN_DBG="off"
	CONFIG_DIS_INSN_DBG="off"
	CONFIG_PVL_DBG="off"
	CONFIG_INSN_DBG="off"
	CONFIG_CPUID_DBG="off"
	CONFIG_CR_DBG="off"
	CONFIG_DR_DBG="off"
	CONFIG_MSR_DBG="off"
	CONFIG_IRQ_DBG="off"
	CONFIG_INT_DBG="off"
	CONFIG_IO_DBG="off"
	CONFIG_EXCP_DBG="off"
	CONFIG_GP_DBG="off"
	CONFIG_BP_DBG="off"
	CONFIG_DB_DBG="off"
	CONFIG_PF_DBG="off"
	CONFIG_PF_VERBOSE_DBG="off"
	CONFIG_SMI_DBG="off"
	CONFIG_CTRL_DBG="off"
	CONFIG_GDB_DBG="off"
	CONFIG_GDB_PKT_DBG="off"
	CONFIG_GDB_CMD_DBG="off"
	CONFIG_GDB_PARSE_DBG="off"
	CONFIG_DEV_DBG="off"
	CONFIG_DEV_PIC_DBG="off"
	CONFIG_DEV_UART_DBG="off"
	CONFIG_DEV_IO_DBG="off"
	CONFIG_DEV_PS2_DBG="off"
	CONFIG_DEV_KBD_DBG="off"
	CONFIG_SVM_DBG="off"
	CONFIG_SVM_CPU_DBG="off"
	CONFIG_SVM_IDT_DBG="off"
	CONFIG_SVM_EXCP_DBG="off"
	CONFIG_SVM_EXCP_GP_DBG="off"
	CONFIG_SVM_EXCP_PF_DBG="off"
	CONFIG_SVM_NPF_DBG="off"
	CONFIG_SVM_INT_DBG="off"
	CONFIG_SVM_IO_DBG="off"
	CONFIG_SVM_IRQ_DBG="off"
	CONFIG_SVM_CPUID_DBG="off"
	CONFIG_SVM_MSR_DBG="off"
	CONFIG_SVM_CR_DBG="off"
	CONFIG_VMX_DBG="off"
	CONFIG_VMX_CPU_DBG="off"
	CONFIG_VMX_IDT_DBG="off"
	CONFIG_VMX_EPT_DBG="off"
	CONFIG_VMX_EXCP_DBG="off"
	CONFIG_VMX_EXCP_PF_DBG="off"
	CONFIG_VMX_EXCP_GP_DBG="off"
	CONFIG_VMX_INT_DBG="off"
	CONFIG_VMX_IO_DBG="off"
	CONFIG_VMX_IRQ_DBG="off"
	CONFIG_VMX_CPUID_DBG="off"
	CONFIG_VMX_MSR_DBG="off"
	CONFIG_VMX_CR_DBG="off"

	for v in $ans;do
	    eval $v="on"
	done
    fi

    # enforce vmm to debug nothing, but let setup print out some stuff
    if [ $CONFIG_PRINT == "none" ];then
	$CONFIG_DEBUG_ACTIVE = "off"
    fi
}

function commit()
{
    confout="${conf}" ; :>$confout

    echo "CONFIG_ARCH=$CONFIG_ARCH" >> $confout
    echo "CONFIG_CTRL=$CONFIG_CTRL" >> $confout
    echo "CONFIG_PRINT=$CONFIG_PRINT" >> $confout
    echo "CONFIG_MSR_PROXY=$CONFIG_MSR_PROXY" >> $confout
    echo "CONFIG_EHCI_FULL=$CONFIG_EHCI_FULL" >> $confout
    echo "CONFIG_EHCI_2ND=$CONFIG_EHCI_2ND" >> $confout
    echo "CONFIG_INST_DIR=$CONFIG_INST_DIR" >> $confout

    echo "CONFIG_DEBUG_ACTIVE=$CONFIG_DEBUG_ACTIVE" >> $confout
    echo "CONFIG_VMEXIT_TRACE=$CONFIG_VMEXIT_TRACE" >> $confout

    echo "CONFIG_VMM_DBG=$CONFIG_VMM_DBG" >> $confout
    echo "CONFIG_VM_ACCESS_DBG=$CONFIG_VM_ACCESS_DBG" >> $confout
    echo "CONFIG_MP_DBG=$CONFIG_MP_DBG" >> $confout
    echo "CONFIG_PG_DBG=$CONFIG_PG_DBG" >> $confout
    echo "CONFIG_PG_W_DBG=$CONFIG_PG_W_DBG" >> $confout
    echo "CONFIG_SMAP_DBG=$CONFIG_SMAP_DBG" >> $confout
    echo "CONFIG_PMEM_DBG=$CONFIG_PMEM_DBG" >> $confout
    echo "CONFIG_VMEM_DBG=$CONFIG_VMEM_DBG" >> $confout
    echo "CONFIG_CPU_DBG=$CONFIG_CPU_DBG" >> $confout
    echo "CONFIG_EHCI_DBG=$CONFIG_EHCI_DBG" >> $confout
    echo "CONFIG_UART_DBG=$CONFIG_UART_DBG" >> $confout
    echo "CONFIG_EMU_DBG=$CONFIG_EMU_DBG" >> $confout
    echo "CONFIG_DIS_DBG=$CONFIG_DIS_DBG" >> $confout
    echo "CONFIG_EMU_INSN_DBG=$CONFIG_EMU_INSN_DBG" >> $confout
    echo "CONFIG_DIS_INSN_DBG=$CONFIG_DIS_INSN_DBG" >> $confout
    echo "CONFIG_PVL_DBG=$CONFIG_PVL_DBG" >> $confout
    echo "CONFIG_INSN_DBG=$CONFIG_INSN_DBG" >> $confout
    echo "CONFIG_CPUID_DBG=$CONFIG_CPUID_DBG" >> $confout
    echo "CONFIG_CR_DBG=$CONFIG_CR_DBG" >> $confout
    echo "CONFIG_DR_DBG=$CONFIG_DR_DBG" >> $confout
    echo "CONFIG_MSR_DBG=$CONFIG_MSR_DBG" >> $confout
    echo "CONFIG_IRQ_DBG=$CONFIG_IRQ_DBG" >> $confout
    echo "CONFIG_INT_DBG=$CONFIG_INT_DBG" >> $confout
    echo "CONFIG_IO_DBG=$CONFIG_IO_DBG" >> $confout
    echo "CONFIG_EXCP_DBG=$CONFIG_EXCP_DBG" >> $confout
    echo "CONFIG_GP_DBG=$CONFIG_GP_DBG" >> $confout
    echo "CONFIG_BP_DBG=$CONFIG_BP_DBG" >> $confout
    echo "CONFIG_DB_DBG=$CONFIG_DB_DBG" >> $confout
    echo "CONFIG_PF_DBG=$CONFIG_PF_DBG" >> $confout
    echo "CONFIG_PF_VERBOSE_DBG=$CONFIG_PF_VERBOSE_DBG" >> $confout
    echo "CONFIG_SMI_DBG=$CONFIG_SMI_DBG" >> $confout
    echo "CONFIG_CTRL_DBG=$CONFIG_CTRL_DBG" >> $confout
    echo "CONFIG_GDB_DBG=$CONFIG_GDB_DBG" >> $confout
    echo "CONFIG_GDB_PKT_DBG=$CONFIG_GDB_PKT_DBG" >> $confout
    echo "CONFIG_GDB_CMD_DBG=$CONFIG_GDB_CMD_DBG" >> $confout
    echo "CONFIG_GDB_PARSE_DBG=$CONFIG_GDB_PARSE_DBG" >> $confout
    echo "CONFIG_DEV_DBG=$CONFIG_DEV_DBG" >> $confout
    echo "CONFIG_DEV_PIC_DBG=$CONFIG_DEV_PIC_DBG" >> $confout
    echo "CONFIG_DEV_UART_DBG=$CONFIG_DEV_UART_DBG" >> $confout
    echo "CONFIG_DEV_IO_DBG=$CONFIG_DEV_IO_DBG" >> $confout
    echo "CONFIG_DEV_PS2_DBG=$CONFIG_DEV_PS2_DBG" >> $confout
    echo "CONFIG_DEV_KBD_DBG=$CONFIG_DEV_KBD_DBG" >> $confout
    echo "CONFIG_SVM_DBG=$CONFIG_SVM_DBG" >> $confout
    echo "CONFIG_SVM_CPU_DBG=$CONFIG_SVM_CPU_DBG" >> $confout
    echo "CONFIG_SVM_IDT_DBG=$CONFIG_SVM_IDT_DBG" >> $confout
    echo "CONFIG_SVM_EXCP_DBG=$CONFIG_SVM_EXCP_DBG" >> $confout
    echo "CONFIG_SVM_EXCP_GP_DBG=$CONFIG_SVM_EXCP_GP_DBG" >> $confout
    echo "CONFIG_SVM_EXCP_PF_DBG=$CONFIG_SVM_EXCP_PF_DBG" >> $confout
    echo "CONFIG_SVM_NPF_DBG=$CONFIG_SVM_NPF_DBG" >> $confout
    echo "CONFIG_SVM_INT_DBG=$CONFIG_SVM_INT_DBG" >> $confout
    echo "CONFIG_SVM_IO_DBG=$CONFIG_SVM_IO_DBG" >> $confout
    echo "CONFIG_SVM_IRQ_DBG=$CONFIG_SVM_IRQ_DBG" >> $confout
    echo "CONFIG_SVM_CPUID_DBG=$CONFIG_SVM_CPUID_DBG" >> $confout
    echo "CONFIG_SVM_MSR_DBG=$CONFIG_SVM_MSR_DBG" >> $confout
    echo "CONFIG_SVM_CR_DBG=$CONFIG_SVM_CR_DBG" >> $confout
    echo "CONFIG_VMX_DBG=$CONFIG_VMX_DBG" >> $confout
    echo "CONFIG_VMX_CPU_DBG=$CONFIG_VMX_CPU_DBG" >> $confout
    echo "CONFIG_VMX_IDT_DBG=$CONFIG_VMX_IDT_DBG" >> $confout
    echo "CONFIG_VMX_EPT_DBG=$CONFIG_VMX_EPT_DBG" >> $confout
    echo "CONFIG_VMX_EXCP_DBG=$CONFIG_VMX_EXCP_DBG" >> $confout
    echo "CONFIG_VMX_EXCP_PF_DBG=$CONFIG_VMX_EXCP_PF_DBG" >> $confout
    echo "CONFIG_VMX_EXCP_GP_DBG=$CONFIG_VMX_EXCP_GP_DBG" >> $confout
    echo "CONFIG_VMX_INT_DBG=$CONFIG_VMX_INT_DBG" >> $confout
    echo "CONFIG_VMX_IO_DBG=$CONFIG_VMX_IO_DBG" >> $confout
    echo "CONFIG_VMX_IRQ_DBG=$CONFIG_VMX_IRQ_DBG" >> $confout
    echo "CONFIG_VMX_CPUID_DBG=$CONFIG_VMX_CPUID_DBG" >> $confout
    echo "CONFIG_VMX_MSR_DBG=$CONFIG_VMX_MSR_DBG" >> $confout
    echo "CONFIG_VMX_CR_DBG=$CONFIG_VMX_CR_DBG" >> $confout
}

#
# Main
#
reading

while /bin/true;
do
    $dialog --menu "" 10 50 10 \
	"build" "change build settings" \
	"debug" "select debug info" \
	"install" "change install directory" \
	"quit" "save configuration and quit"

    if [ $? -ne 0 ];then
	break
    fi

    ans=$(<$answer) ; :>$answer
    if [ $ans == "build" ];then
	building
    elif [ $ans == "debug" ];then
	debuging
    elif [ $ans == "install" ];then
	installing
    elif [ $ans == "quit" ];then
	commit
	break
    fi
done

rm -f $answer