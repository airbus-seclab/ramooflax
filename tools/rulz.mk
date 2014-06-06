#############################################################################
######################### You should not modify ! ###########################
#############################################################################

define pre-inst
vmware-mount "/home/stf/vmware/Debian 7 64-bit/Debian 7 64-bit.vmdk" 1 "/home/stf/vmware/Debian 7 64-bit/mnt/"
endef

define inst
$(CP) $@ "/home/stf/vmware/Debian 7 64-bit/mnt/boot/"$(notdir $@)
echo "    CP    $@"
endef

define post-inst
vmware-mount -X
endef

define __pre-inst
mount $(CONFIG_INST_DIR)
endef

define __inst
$(CP) $@ $(CONFIG_INST_DIR)/$(notdir $@)
echo "    CP    $@"
endef

define __post-inst
umount -l $(CONFIG_INST_DIR)
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
