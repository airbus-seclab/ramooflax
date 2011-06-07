#!/usr/bin/make -f

SUBDIRS := core drivers libc devices

ifneq ($(USE_AMD),)
SUBDIRS += svm
else
SUBDIRS += vmx
endif

LD            := $(LD_x64)
CCLIB         := $(CCLIB_x64)
EXTRA_CFLAGS  := -D__INIT__ -D__X86_64__ $(CFLG_x64)
