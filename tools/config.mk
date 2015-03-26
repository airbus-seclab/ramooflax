#############################################################################
######################### You should not modify ! ###########################
#############################################################################

MAKEFLAGS  := --no-print-directory

PROJECTS   := loader setup vmm
SRC_DIR    := src
BIN_DIR    := build
CONFTOOL   := tools/config.py
CONFIG     := .config

CC_MODEL   := small
CCVER      := 4.8
CC         := $(shell which gcc-$(CCVER))
CPP        := $(shell which cpp-$(CCVER))
LD         := $(shell which ld)
RM         := $(shell which rm)
CP         := $(shell which cp)
SED        := $(shell which sed)
FIND       := $(shell which find)
INSTOOL    := tools/installer.sh

CFLG_WRN   := -Wall -W
CFLG_KRN   := -pipe -nostdlib -nostdinc -ffreestanding -fms-extensions
CFLG_FP    := -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4.1 \
              -mno-sse4.2 -mno-sse4 -mno-avx -mno-avx2 -mno-aes -mno-pclmul \
              -mno-fsgsbase -mno-rdrnd -mno-f16c -mno-fma -mno-sse4a \
              -mno-fma4 -mno-xop -mno-lwp -mno-3dnow -mno-popcnt \
              -mno-abm -mno-bmi -mno-bmi2 -mno-lzcnt -mno-tbm

CFLG_32    := -m32
CCLIB_32   := $(shell $(CC) -m32 -print-libgcc-file-name)
LDFLG_32   := -melf_i386

CFLG_64    := -m64 -mno-red-zone -mcmodel=$(CC_MODEL) -D__X86_64__ $(CFLG_FP)
CCLIB_64   := $(shell $(CC) -m64 -print-libgcc-file-name)
LDFLG_64   := -melf_x86_64

CFLAGS     := $(CFLG_WRN) $(CFLG_KRN)
LDFLAGS    := --warn-common --no-check-sections -n
