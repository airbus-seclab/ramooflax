#!/usr/bin/make -f

SUBDIRS := core drivers libc devices disasm control

ifneq ($(USE_AMD),)
SUBDIRS += svm
else
SUBDIRS += vmx
endif

LD            := $(LD_x64)
CCLIB         := $(CCLIB_x64)
EXTRA_CFLAGS  := -fpie -D__X86_64__ $(CFLG_x64)
EXTRA_LDFLAGS := -pie
