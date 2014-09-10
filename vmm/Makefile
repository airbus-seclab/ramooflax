#!/usr/bin/make -f

SUBDIRS := core libc devices disasm control

ifeq ($(CONFIG_HAS_DRV),yes)
SUBDIRS += drivers
endif

ifeq ($(CONFIG_HAS_NET),yes)
SUBDIRS += net
endif

ifeq ($(CONFIG_ARCH),amd)
SUBDIRS += svm
else
SUBDIRS += vmx
endif

CCLIB          := $(CCLIB_64)
EXTRA_CFLAGS   := $(CFLG_64) -fpie
EXTRA_LDFLAGS  := $(LDFLG_64)
EXTRA2_LDFLAGS := -pie
