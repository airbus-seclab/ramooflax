obj := dummy.o

ifeq ($(CONFIG_HAS_EHCI),yes)
obj += ehci.o
endif

ifeq ($(CONFIG_HAS_E1000),yes)
obj += e1000.o
endif

ifeq ($(CONFIG_HAS_UART),yes)
obj += uart.o
endif
