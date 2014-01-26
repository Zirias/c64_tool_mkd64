P := modules$(PSEP)perl$(PSEP)

perl_OBJS := $(P)module.o $(P)perlxsi.o

perl_SOURCES := $(perl_OBJS:.o=.c)
perl_CLEAN := $(P)buildid.h $(P)perlxsi.c

ifdef PERL
perl_CFLAGS := $(shell perl -MExtUtils::Embed -e ccopts) \
	-DPERL_GCC_PEDANTIC -DPERL_NO_SHORT_NAMES
perl_LDFLAGS := $(shell perl -MExtUtils::Embed -e ldopts)

MODULES += $(OUTDIR)$(PSEP)perl$(SO)
endif

SOURCES += $(perl_SOURCES)
CLEAN += $(perl_CLEAN)

ifdef PERL
$(P)buildid.h: $(perl_SOURCES) Makefile conf.mk | $(BID)
	$(VGEN)
	$(VR)$(BID) > $@

$(P)perlxsi.c:
	$(VGEN)
	$(VR)perl -MExtUtils::Embed -e xsinit -- -o - | \
	       perl -e 'while(<STDIN>){s/newXS\(/Perl_newXS(aTHX_ /;print}' >$@

$(OUTDIR)$(PSEP)perl$(SO): $(perl_OBJS) $(mod_LIBS) | outdir
	$(VLD)
	$(VR)$(CC) -shared -o$@ $(LDFLAGS) $(perl_LDFLAGS) $^

$(P)%.d: $(P)%.c Makefile $(P)module.mk conf.mk | $(P)buildid.h
	$(VDEP)
	$(VR)$(CCDEP) -MT"$@ $(@:.d=.o)" -MF$@ \
		$(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $(perl_CFLAGS) $<

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include $(perl_OBJS:.o=.d)
endif
endif

$(P)%.o: $(P)%.c Makefile $(P)module.mk conf.mk
	$(VCC)
	$(VR)$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $(perl_CFLAGS) $<
endif

# vim: noet:si:ts=8:sts=8:sw=8
