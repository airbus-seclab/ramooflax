#############################################################################
###################### Commit configuration to CFLAGS #######################
#############################################################################
CFLG_CFG :=

ifeq ($(CONFIG_ARCH),amd)
USE_AMD := 1
CFLG_CONFIG += -D__SVM__
endif

ifeq ($(CONFIG_CTRL),ehci)
USE_EHCI := 1
CFLG_CONFIG += -D__EHCI_CTRL__
endif

ifeq ($(CONFIG_CTRL),uart)
USE_UART := 1
CFLG_CONFIG += -D__UART_CTRL__
endif

ifeq ($(CONFIG_PRINT),ehci)
USE_EHCI := 1
CFLG_CONFIG += -D__EHCI_PRINT__
endif

ifeq ($(CONFIG_PRINT),uart)
USE_UART := 1
CFLG_CONFIG += -D__UART_PRINT__
endif

ifeq ($(CONFIG_MSR_PROXY),on)
CFLG_CONFIG += -D__MSR_PROXY__
endif

ifeq ($(USE_EHCI),1)
ifeq ($(CONFIG_EHCI_FULL),on)
CFLG_CONFIG += -D__EHCI_FULL__
endif
ifeq ($(CONFIG_EHCI_2ND),on)
CFLG_CONFIG += -D__EHCI_2ND__
endif
endif

ifeq ($(CONFIG_DEBUG_ACTIVE),on)
CFLG_CONFIG += -D__DEBUG_ACTIVE__
endif
ifeq ($(CONFIG_VMEXIT_TRACE),on)
CFLG_CONFIG += -D__DEBUG_VMEXIT_TRACE__
endif
ifeq ($(CONFIG_VMM_DBG),on)
CFLG_CONFIG += -DVMM_DBG
endif
ifeq ($(CONFIG_VM_ACCESS_DBG),on)
CFLG_CONFIG += -DVM_ACCESS_DBG
endif
ifeq ($(CONFIG_MP_DBG),on)
CFLG_CONFIG += -DMP_DBG
endif
ifeq ($(CONFIG_PG_DBG),on)
CFLG_CONFIG += -DPG_DBG
endif
ifeq ($(CONFIG_PG_W_DBG),on)
CFLG_CONFIG += -DPG_W_DBG
endif
ifeq ($(CONFIG_SMAP_DBG),on)
CFLG_CONFIG += -DSMAP_DBG
endif
ifeq ($(CONFIG_PMEM_DBG),on)
CFLG_CONFIG += -DPMEM_DBG
endif
ifeq ($(CONFIG_VMEM_DBG),on)
CFLG_CONFIG += -DVMEM_DBG
endif
ifeq ($(CONFIG_CPU_DBG),on)
CFLG_CONFIG += -DCPU_DBG
endif
ifeq ($(CONFIG_EHCI_DBG),on)
CFLG_CONFIG += -DEHCI_COND_DBG
endif
ifeq ($(CONFIG_UART_DBG),on)
CFLG_CONFIG += -DUART_COND_DBG
endif
ifeq ($(CONFIG_EMU_DBG),on)
CFLG_CONFIG += -DEMU_DBG
endif
ifeq ($(CONFIG_DIS_DBG),on)
CFLG_CONFIG += -DDIS_DBG
endif
ifeq ($(CONFIG_EMU_INSN_DBG),on)
CFLG_CONFIG += -DEMU_INSN_DBG
endif
ifeq ($(CONFIG_DIS_INSN_DBG),on)
CFLG_CONFIG += -DDIS_INSN_DBG
endif
ifeq ($(CONFIG_PVL_DBG),on)
CFLG_CONFIG += -DPVL_DBG
endif
ifeq ($(CONFIG_INSN_DBG),on)
CFLG_CONFIG += -DINSN_DBG
endif
ifeq ($(CONFIG_CPUID_DBG),on)
CFLG_CONFIG += -DCPUID_DBG
endif
ifeq ($(CONFIG_CR_DBG),on)
CFLG_CONFIG += -DCR_DBG
endif
ifeq ($(CONFIG_DR_DBG),on)
CFLG_CONFIG += -DDR_DBG
endif
ifeq ($(CONFIG_MSR_DBG),on)
CFLG_CONFIG += -DMSR_DBG
endif
ifeq ($(CONFIG_IRQ_DBG),on)
CFLG_CONFIG += -DIRQ_DBG
endif
ifeq ($(CONFIG_INT_DBG),on)
CFLG_CONFIG += -DINT_DBG
endif
ifeq ($(CONFIG_IO_DBG),on)
CFLG_CONFIG += -DIO_DBG
endif
ifeq ($(CONFIG_EXCP_DBG),on)
CFLG_CONFIG += -DEXCP_DBG
endif
ifeq ($(CONFIG_GP_DBG),on)
CFLG_CONFIG += -DGP_DBG
endif
ifeq ($(CONFIG_BP_DBG),on)
CFLG_CONFIG += -DBP_DBG
endif
ifeq ($(CONFIG_DB_DBG),on)
CFLG_CONFIG += -DDB_DBG
endif
ifeq ($(CONFIG_PF_DBG),on)
CFLG_CONFIG += -DPF_DBG
endif
ifeq ($(CONFIG_PF_VERBOSE_DBG),on)
CFLG_CONFIG += -DPF_VERBOSE_DBG
endif
ifeq ($(CONFIG_SMI_DBG),on)
CFLG_CONFIG += -DSMI_DBG
endif
ifeq ($(CONFIG_CTRL_DBG),on)
CFLG_CONFIG += -DCTRL_DBG
endif
ifeq ($(CONFIG_GDB_DBG),on)
CFLG_CONFIG += -DGDB_DBG
endif
ifeq ($(CONFIG_GDB_PKT_DBG),on)
CFLG_CONFIG += -DGDB_PKT_DBG
endif
ifeq ($(CONFIG_GDB_CMD_DBG),on)
CFLG_CONFIG += -DGDB_CMD_DBG
endif
ifeq ($(CONFIG_GDB_PARSE_DBG),on)
CFLG_CONFIG += -DGDB_PARSE_DBG
endif
ifeq ($(CONFIG_DEV_DBG),on)
CFLG_CONFIG += -DDEV_DBG
endif
ifeq ($(CONFIG_DEV_PIC_DBG),on)
CFLG_CONFIG += -DDEV_PIC_DBG
endif
ifeq ($(CONFIG_DEV_UART_DBG),on)
CFLG_CONFIG += -DDEV_UART_DBG
endif
ifeq ($(CONFIG_DEV_IO_DBG),on)
CFLG_CONFIG += -DDEV_IO_DBG
endif
ifeq ($(CONFIG_DEV_PS2_DBG),on)
CFLG_CONFIG += -DDEV_PS2_DBG
endif
ifeq ($(CONFIG_DEV_KBD_DBG),on)
CFLG_CONFIG += -DDEV_KBD_DBG
endif
ifeq ($(CONFIG_SVM_DBG),on)
CFLG_CONFIG += -DSVM_DBG
endif
ifeq ($(CONFIG_SVM_CPU_DBG),on)
CFLG_CONFIG += -DSVM_CPU_DBG
endif
ifeq ($(CONFIG_SVM_IDT_DBG),on)
CFLG_CONFIG += -DSVM_IDT_DBG
endif
ifeq ($(CONFIG_SVM_EXCP_DBG),on)
CFLG_CONFIG += -DSVM_EXCP_DBG
endif
ifeq ($(CONFIG_SVM_EXCP_GP_DBG),on)
CFLG_CONFIG += -DSVM_EXCP_GP_DBG
endif
ifeq ($(CONFIG_SVM_EXCP_PF_DBG),on)
CFLG_CONFIG += -DSVM_EXCP_PF_DBG
endif
ifeq ($(CONFIG_SVM_NPF_DBG),on)
CFLG_CONFIG += -DSVM_NPF_DBG
endif
ifeq ($(CONFIG_SVM_INT_DBG),on)
CFLG_CONFIG += -DSVM_INT_DBG
endif
ifeq ($(CONFIG_SVM_IO_DBG),on)
CFLG_CONFIG += -DSVM_IO_DBG
endif
ifeq ($(CONFIG_SVM_IRQ_DBG),on)
CFLG_CONFIG += -DSVM_IRQ_DBG
endif
ifeq ($(CONFIG_SVM_CPUID_DBG),on)
CFLG_CONFIG += -DSVM_CPUID_DBG
endif
ifeq ($(CONFIG_SVM_MSR_DBG),on)
CFLG_CONFIG += -DSVM_MSR_DBG
endif
ifeq ($(CONFIG_SVM_CR_DBG),on)
CFLG_CONFIG += -DSVM_CR_DBG
endif
ifeq ($(CONFIG_VMX_DBG),on)
CFLG_CONFIG += -DVMX_DBG
endif
ifeq ($(CONFIG_VMX_CPU_DBG),on)
CFLG_CONFIG += -DVMX_CPU_DBG
endif
ifeq ($(CONFIG_VMX_IDT_DBG),on)
CFLG_CONFIG += -DVMX_IDT_DBG
endif
ifeq ($(CONFIG_VMX_EPT_DBG),on)
CFLG_CONFIG += -DVMX_EPT_DBG
endif
ifeq ($(CONFIG_VMX_EXCP_DBG),on)
CFLG_CONFIG += -DVMX_EXCP_DBG
endif
ifeq ($(CONFIG_VMX_EXCP_PF_DBG),on)
CFLG_CONFIG += -DVMX_EXCP_PF_DBG
endif
ifeq ($(CONFIG_VMX_EXCP_GP_DBG),on)
CFLG_CONFIG += -DVMX_EXCP_GP_DBG
endif
ifeq ($(CONFIG_VMX_INT_DBG),on)
CFLG_CONFIG += -DVMX_INT_DBG
endif
ifeq ($(CONFIG_VMX_IO_DBG),on)
CFLG_CONFIG += -DVMX_IO_DBG
endif
ifeq ($(CONFIG_VMX_IRQ_DBG),on)
CFLG_CONFIG += -DVMX_IRQ_DBG
endif
ifeq ($(CONFIG_VMX_CPUID_DBG),on)
CFLG_CONFIG += -DVMX_CPUID_DBG
endif
ifeq ($(CONFIG_VMX_MSR_DBG),on)
CFLG_CONFIG += -DVMX_MSR_DBG
endif
ifeq ($(CONFIG_VMX_CR_DBG),on)
CFLG_CONFIG += -DVMX_CR_DBG
endif

INST_DIR   := $(CONFIG_INST_DIR)
CFLAGS     += $(CFLG_CONFIG)
