obj := dummy.o

ifeq ($(CONFIG_HAS_VIDEO),yes)
obj += video.o
endif

ifeq ($(CONFIG_HAS_EHCI),yes)
obj += pci.o pci_dbgp.o ehci.o
endif

ifeq ($(CONFIG_HAS_E1000),yes)
obj += pci.o pci_e1000.o e1000.o
endif

ifeq ($(CONFIG_HAS_UART),yes)
obj += uart.o
endif
