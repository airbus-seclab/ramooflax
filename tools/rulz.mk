#############################################################################
######################### You should not modify ! ###########################
#############################################################################

define pre-inst
mount $(INST_DIR)
endef

define inst
$(CP) $@ $(INST_DIR)/$(notdir $@)
echo "    CP    $@"
endef

define post-inst
umount -l $(INST_DIR)
endef

define compile
echo "    CC    $<"
$(CC) $(INCLUDE) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<
endef

define assemble
echo "    AS    $<"
$(CPP) $< $(CFLAGS) $(EXTRA_CFLAGS) -o $<.s
$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ -c $<.s 
$(RM) $<.s
endef

define depend
echo "    DP    $<"
$(CPP) -M -MG -MT '$<' $(INCLUDE) $(CFLAGS) $(EXTRA_CFLAGS) $< | \
$(SED) 's,\($*\)\.[c|s][ :]*,\1.o $@ : ,' > $@
endef

define aggregate
echo "    LD    $@"
$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) -r -o $@ $^
endef

define link
echo "    LD    $@"
$(LD) $(LDFLAGS) $(EXTRA_LDFLAGS) $(EXTRA2_LDFLAGS) -T $(LDSCRIPT) $^ -o $@ $(CCLIB)
endef

define config
$(CONFTOOL) $@
endef

%.d: %.c
	@$(depend)
%.d: %.s
	@$(depend)
%.o: %.c
	@$(compile)
%.o: %.s
	@$(assemble)
