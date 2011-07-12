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

INST_DIR   := $(CONFIG_INST_DIR)
CFLAGS     += $(CFLG_CONFIG)
