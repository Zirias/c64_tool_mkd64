P := modules$(PSEP)cbmdos$(PSEP)

cbmdos_OBJS := $(P)module.o $(P)alloc.o 

cbmdos_SOURCES := $(cbmdos_OBJS:.o=.c)
cbmdos_CLEAN := $(P)buildid.h

SOURCES += $(cbmdos_SOURCES)
MODULES += $(OUTDIR)$(PSEP)cbmdos$(SO)
CLEAN += $(cbmdos_CLEAN)

$(P)buildid.h: $(cbmdos_SOURCES) Makefile conf.mk | $(BID)
	$(VGEN)
	$(VR)$(BID) > $@

$(OUTDIR)$(PSEP)cbmdos$(SO): $(cbmdos_OBJS) $(mod_LIBS) | outdir
	$(VLD)
	$(VR)$(CC) -shared -o$@ $(LDFLAGS) $^

$(P)%.d: $(P)%.c Makefile conf.mk | $(P)buildid.h
	$(VDEP)
	$(VR)$(CCDEP) -MT"$@ $(@:.d=.o)" -MF$@ \
		$(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include $(cbmdos_OBJS:.o=.d)
endif
endif

$(P)%.o: $(P)%.c Makefile conf.mk
	$(VCC)
	$(VR)$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

# vim: noet:si:ts=8:sts=8:sw=8
