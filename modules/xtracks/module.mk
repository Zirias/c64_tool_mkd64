P := modules$(PSEP)xtracks$(PSEP)

xtracks_OBJS := $(P)module.o

xtracks_SOURCES := $(xtracks_OBJS:.o=.c)
xtracks_CLEAN := $(P)buildid.h

SOURCES += $(xtracks_SOURCES)
MODULES += $(OUT)$(PSEP)xtracks$(SO)
CLEAN += $(xtracks_CLEAN)

$(P)buildid.h: $(xtracks_SOURCES) Makefile conf.mk | $(BID)
	$(VGEN)
	$(VR)$(BID) > $@

$(OUT)$(PSEP)xtracks$(SO): $(xtracks_OBJS) $(mod_LIBS) | outdir
	$(VLD)
	$(VR)$(CC) -shared -o$@ $(LDFLAGS) $^

$(P)%.d: $(P)%.c Makefile conf.mk | $(P)buildid.h
	$(VDEP)
	$(VR)$(CCDEP) -MT"$@ $(@:.d=.o)" -MF$@ \
		$(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

-include $(xtracks_OBJS:.o=.d)

$(P)%.o: $(P)%.c Makefile conf.mk
	$(VCC)
	$(VR)$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

# vim: noet:si:ts=8:sts=8:sw=8
