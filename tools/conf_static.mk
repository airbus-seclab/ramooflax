#############################################################################
######################### You should not modify ! ###########################
#############################################################################

MAKEFLAGS  := --no-print-directory

PROJECTS   := loader setup vmm
SRC_DIR    := src
BIN_DIR    := build
CONFTOOL   := tools/menuconfig.sh
CONFIG     := .config

CC_MODEL   := small
CCVER      := 4.4
CC         := $(shell which gcc-$(CCVER))
CPP        := $(shell which cpp-$(CCVER))
LD         := $(shell which ld)
LD_x64     := $(shell which x86_64-linux-gnu-ld)
RM         := $(shell which rm)
CP         := $(shell which cp)
SED        := $(shell which sed)
FIND       := $(shell which find)
CCLIB      := $(shell $(CC) -m32 -print-libgcc-file-name)
CCLIB_x64  := $(shell $(CC) -m64 -print-libgcc-file-name)

CFLG_x64   := -m64 -mno-red-zone -mcmodel=$(CC_MODEL)
CFLG_WRN   := -Wall -W
CFLG_KRN   := -pipe -nostdlib -nostdinc -ffreestanding -fms-extensions
CFLG_FP    := -mno-sse -mno-mmx -mno-sse2 -mno-3dnow

CFLAGS     := $(CFLG_WRN) $(CFLG_KRN)
LDFLAGS    := --warn-common --no-check-sections -n
