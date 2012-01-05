obj := video.o

ifeq ($(CONFIG_HAS_EHCI),yes)
obj += pci.o ehci.o
endif

ifeq ($(CONFIG_HAS_UART),yes)
obj += uart.o
endif
