#!/usr/bin/make -f

include tools/config.mk

.PHONY: clean $(PROJECTS)

ifneq ($(cfg),)
.PHONY: $(CONFIG)
endif

ifeq ($(act),)
all: $(PROJECTS)
install: $(PROJECTS)
	@$(MAKE) act=$@
clean:
	@$(MAKE) act=$@
distclean: clean
	@$(RM) -f $(CONFIG)
config:
	@$(MAKE) cfg=force $(CONFIG)
$(CONFIG):
	@$(CONFTOOL) $@
$(PROJECTS): $(CONFIG)
	@$(MAKE) act=build tgt=$@
else
ifeq ($(tgt),)
all: $(PROJECTS)
$(PROJECTS):
	@$(MAKE) act=$(act) tgt=$@
else
tbase := $(tgt)/$(BIN_DIR)/$(tgt)
tbin  := $(tbase).bin
.PHONY: $(tbin)

ifeq ($(act),clean)
$(tbin):
	@$(RM) -f $@
	@$(FIND) $(tgt) -name \*.[od] | xargs $(RM) -f
else
include $(CONFIG)
include tools/rulz.mk

ifeq ($(act),install)
$(tbin):
	@$(pre-inst)
	@$(inst)
	@$(post-inst)
else
INCLUDE := -I$(tgt)/include -Iinclude

ifeq ($(blob),)
include $(tgt)/Makefile
subdirs  := $(addprefix $(tgt)/$(SRC_DIR)/,$(SUBDIRS))
blobs    := $(addsuffix /build.o, $(subdirs))
LDSCRIPT := $(tbase).lds
.PHONY: $(blobs)
$(tbin): $(blobs)
	@$(link)
$(blobs):
	@$(MAKE) act=$(act) tgt=$(tgt) blob=$@
else
blob_path := $(dir $(blob))
include $(blob_path)/Makefile
include $(tgt)/Makefile
blob_obj := $(addprefix $(blob_path), $(obj))
blob_dep := $(blob_obj:.o=.d)
$(blob): $(blob_obj)
	@$(aggregate)
-include $(blob_dep)
endif #blob
endif #act=install
endif #act=clean
endif #tgt
endif #act
