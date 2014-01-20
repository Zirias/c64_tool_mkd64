P := modules$(PSEP)sepgen$(PSEP)

sepgen_OBJS := $(P)module.o

sepgen_SOURCES := $(sepgen_OBJS:.o=.c)
sepgen_CLEAN := $(P)buildid.h

SOURCES += $(sepgen_SOURCES)
MODULES += $(OUT)$(PSEP)sepgen$(SO)
CLEAN += $(sepgen_CLEAN)

$(P)buildid.h: $(sepgen_SOURCES) Makefile conf.mk | $(BID)
	$(VGEN)
	$(VR)$(BID) > $@

$(OUT)$(PSEP)sepgen$(SO): $(sepgen_OBJS) $(mod_LIBS) | outdir
	$(VLD)
	$(VR)$(CC) -shared -o$@ $(LDFLAGS) $^

$(P)%.d: $(P)%.c Makefile conf.mk | $(P)buildid.h
	$(VDEP)
	$(VR)$(CCDEP) -MT"$@ $(@:.d=.o)" -MF$@ \
		$(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include $(sepgen_OBJS:.o=.d)
endif
endif

$(P)%.o: $(P)%.c Makefile conf.mk
	$(VCC)
	$(VR)$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

# vim: noet:si:ts=8:sts=8:sw=8
