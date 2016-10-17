obj := acpi.o

ifeq ($(CONFIG_IOMMU), yes)
obj += iommu.o
endif

ifeq ($(CONFIG_HAS_UART),yes)
obj += sio.o
endif

ifeq ($(CONFIG_HAS_VIDEO),yes)
obj += video.o
endif

ifeq ($(CONFIG_HAS_EHCI),yes)
obj += ehci.o pci_dbgp.o pci.o
endif

ifeq ($(CONFIG_HAS_E1000),yes)
obj += e1000.o
endif

ifeq ($(CONFIG_HAS_UART),yes)
obj += uart.o
endif
