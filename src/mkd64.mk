P := src$(PSEP)
PP := src$(PSEP)platform$(PSEP)$(PLATFORM)$(PSEP)

mkd64_OBJS := $(P)mkd64.o $(P)image.o $(P)track.o $(P)block.o $(P)defalloc.o \
	$(P)filemap.o $(P)diskfile.o $(P)cmdline.o $(P)modrepo.o $(P)util.o \
	$(PP)util.o

mkd64_LDFLAGS := $(dl_LDFLAGS)
mkd64_DEFINES := -DBUILDING_MKD64

mkd64_SOURCES := $(mkd64_OBJS:.o=.c)
mkd64_CLEAN := $(P)mkd64.a $(P)buildid.h

SOURCES += $(mkd64_SOURCES)
BINARIES += $(OUT)$(PSEP)mkd64$(EXE)
CLEAN += $(mkd64_CLEAN)

$(P)buildid.h: $(mkd64_SOURCES) Makefile conf.mk | $(BID)
	$(VGEN)
	$(VR)$(BID) > $@

$(OUT)$(PSEP)mkd64$(EXE): $(mkd64_OBJS) | outdir
	$(VLD)
	$(VR)$(CC) -o$@ $(mkd64_LDFLAGS) $(LDFLAGS) $^

$(P)mkd64.a: $(mkd64_OBJS)
	$(VGEN)
	$(VR)-dlltool -l$@ -Dmkd64.exe $^

$(P)%.d: $(P)%.c Makefile conf.mk | $(P)buildid.h
	$(VDEP)
	$(VR)$(CCDEP) -MT"$@ $(@:.d=.o)" -MF$@ \
		$(mkd64_DEFINES) $(CFLAGS) $(INCLUDES) $<

-include $(mkd64_OBJS:.o=.d)

$(P)%.o: $(P)%.c Makefile conf.mk
	$(VCC)
	$(VR)$(CC) -o$@ -c $(mkd64_DEFINES) $(CFLAGS) $(INCLUDES) $<

# vim: noet:si:ts=8:sts=8:sw=8
