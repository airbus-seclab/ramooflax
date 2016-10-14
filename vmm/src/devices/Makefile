obj := io.o ps2.o kbd.o uart.o ata.o

ifeq ($(CONFIG_HAS_UART),yes)
obj += sio.o
endif

ifeq ($(CONFIG_HAS_NET),yes)
obj += pci.o
else ifeq ($(CONFIG_HAS_EHCI),yes)
obj += pci.o
endif
