#!/usr/bin/make -f

SUBDIRS := core drivers libc devices disasm control

ifeq ($(CONFIG_ARCH),amd)
SUBDIRS += svm
else
SUBDIRS += vmx
endif

CCLIB          := $(CCLIB_64)
EXTRA_CFLAGS   := $(CFLG_64) -fpie
EXTRA_LDFLAGS  := $(LDFLG_64)
EXTRA2_LDFLAGS := -pie
