#!/usr/bin/make -f

SUBDIRS := core drivers libc devices

ifeq ($(CONFIG_ARCH),amd)
SUBDIRS += svm
else
SUBDIRS += vmx
endif

CCLIB         := $(CCLIB_64)
EXTRA_CFLAGS  := $(CFLG_64) -D__INIT__
EXTRA_LDFLAGS := $(LDFLG_64)
