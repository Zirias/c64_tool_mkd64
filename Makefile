include defs.mk

CFLAGS += -fvisibility=hidden -std=c99 -Wall -Wextra -pedantic \
	  -Werror=implicit-int \
	  -Werror=implicit-function-declaration \
	  -Werror=declaration-after-statement

LDFLAGS += -static-libgcc

OUTDIR := bin
SDKDIR := sdk
DOCDIR := $(SDKDIR)$(PSEP)doc

VTAGS :=
V := 0

ifdef prefix
bindir := $(prefix)/bin
libbasedir := $(prefix)/lib
libdir := $(libbasedir)/mkd64
includebasedir := $(prefix)/include
includedir := $(includebasedir)/mkd64
docbasedir := $(prefix)/share/doc
docdir := $(docbasedir)/mkd64

INSTALL := install
CFLAGS += -DMODDIR="\"$(libdir)\""
VTAGS += [installable]
else
VTAGS += [portable]
endif

ifdef GCC32
CC := gcc -m32
CFLAGS += -DGCC32BIT
VTAGS += [32bit]
else
CC := gcc
endif

USELTO := 1
ifdef DEBUG
CFLAGS += -DDEBUG -g3 -O0
VTAGS += [debug]
else
CFLAGS += -g0 -O3
ifeq ($(USELTO),1)
CFLAGS += -flto
LDFLAGS += -flto
endif
endif

CCDEP := $(CC) -MM

ifeq ($(V),1)
VCC :=
VDEP :=
VLD :=
VCCLD :=
VGEN :=
VGENT :=
VR :=
else
VCC = @echo $(EQT)   [CC]   $@$(EQT)
VDEP = @echo $(EQT)   [DEP]  $@$(EQT)
VLD = @echo $(EQT)   [LD]   $@$(EQT)
VCCLD = @echo $(EQT)   [CCLD] $@$(EQT)
VGEN = @echo $(EQT)   [GEN]  $@$(EQT)
VGENT = @echo $(EQT)   [GEN]  $@: $(VTAGS)$(EQT)
VR := @
endif

BID := tools$(PSEP)buildid$(EXE)

INCLUDES := -Iinclude

all: bin modules

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
conf.mk:
	$(VGENT)
	$(VR)echo $(EQT)C_CC :=$(CC)$(EQT) >conf.mk
	$(VR)echo $(EQT)C_DEBUG :=$(DEBUG)$(EQT) >>conf.mk
	$(VR)echo $(EQT)C_GCC32 :=$(GCC32)$(EQT) >>conf.mk
	$(VR)echo $(EQT)C_USELTO :=$(USELTO)$(EQT) >>conf.mk
	$(VR)echo $(EQT)C_libdir :=$(libdir)$(EQT) >>conf.mk

-include conf.mk

ifneq ($(strip $(C_CC))_$(strip $(C_DEBUG))_$(strip $(C_GCC32))_$(strip $(C_USELTO))_$(strip $(C_libdir)),$(strip $(CC))_$(strip $(DEBUG))_$(strip $(GCC32))_$(strip $(USELTO))_$(strip $(libdir)))
.PHONY: conf.mk
endif
endif
endif

include src$(PSEP)mkd64.mk
include modules$(PSEP)modules.mk

bin: $(BINARIES)

modules: $(MODULES)

clean:
	$(RMF) $(SOURCES:.c=.o)
	$(RMF) $(SOURCES:.c=.d)
	$(RMF) $(CLEAN)
	$(RMF) $(BID)

distclean: clean
	$(RMF) conf.mk
	$(RMFR) $(SDKDIR) $(CMDQUIET)
	$(RMFR) $(OUTDIR) $(CMDQUIET)

strip: all
	strip --strip-all $(BINARIES)
	strip --strip-unneeded $(MODULES)

ifdef prefix

install: strip
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL) -d $(DESTDIR)$(includedir)
	$(INSTALL) -d $(DESTDIR)$(docdir)/examples/module
	$(INSTALL) $(BINARIES) $(DESTDIR)$(bindir)
	$(INSTALL) $(MODULES) $(DESTDIR)$(libdir)
	$(INSTALL) -m644 include/mkd64/*.h $(DESTDIR)$(includedir)
	$(INSTALL) -m644 README.md $(DESTDIR)$(docdir)
	$(INSTALL) -m644 modapi.txt $(DESTDIR)$(docdir)
	$(INSTALL) -m644 coding.txt $(DESTDIR)$(docdir)
	$(INSTALL) -m644 examples/module/* $(DESTDIR)$(docdir)/examples/module

endif

export DOCDIR
sdkdoc:
	$(MDP) $(DOCDIR) $(CMDQUIET)
	-doxygen api.dox

sdk: bin src$(PSEP)mkd64.a sdkdoc
	$(MDP) $(SDKDIR)$(PSEP)include$(PSEP)mkd64 $(CMDQUIET)
	$(MDP) $(SDKDIR)$(PSEP)lib$(PSEP)mkd64 $(CMDQUIET)
	$(MDP) $(SDKDIR)$(PSEP)examples$(PSEP)module $(CMDQUIET)
	$(CPF) include$(PSEP)mkd64$(PSEP)*.h \
		$(SDKDIR)$(PSEP)include$(PSEP)mkd64
	-$(CPF) src$(PSEP)mkd64.a mkd64sdk$(PSEP)lib$(PSEP)mkd64
	$(CPF) examples$(PSEP)module$(PSEP)Makefile \
		$(SDKDIR)$(PSEP)examples$(PSEP)module
	$(CPF) examples$(PSEP)module$(PSEP)module.c \
		$(SDKDIR)$(PSEP)examples$(PSEP)module
	$(CPF) modapi.txt $(DOCDIR)
	$(CPF) coding.txt $(DOCDIR)

outdir:
	$(VR)$(MDP) $(OUTDIR) $(CMDQUIET)

$(BID): tools$(PSEP)buildid.c $(SOURCES) Makefile conf.mk
	$(VCCLD)
	$(VR)$(CC) -o$@ $(CFLAGS) $<

.PHONY: outdir all bin modules strip clean distclean install sdk sdkdoc
.SUFFIXES:

# vim: noet:si:ts=8:sts=8:sw=8
