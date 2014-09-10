#!/usr/bin/make -f

SUBDIRS := core libc devices

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

CCLIB         := $(CCLIB_64)
EXTRA_CFLAGS  := $(CFLG_64) -D__INIT__
EXTRA_LDFLAGS := $(LDFLG_64)
