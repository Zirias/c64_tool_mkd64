include defs.mk

CFLAGS += -fvisibility=hidden -std=c99 -Wall -Wextra -pedantic \
	  -Werror=implicit-int \
	  -Werror=implicit-function-declaration \
	  -Werror=declaration-after-statement

LDFLAGS += -static-libgcc

OUT := bin

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

ifdef DEBUG
CFLAGS += -DDEBUG -g3 -O0
VTAGS += [debug]
else
CFLAGS += -g0 -O3 -flto
LDFLAGS += -flto
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

conf.mk:
	$(VGENT)
	$(VR)echo $(EQT)C_DEBUG :=$(DEBUG)$(EQT) >conf.mk
	$(VR)echo $(EQT)C_GCC32 :=$(GCC32)$(EQT) >>conf.mk
	$(VR)echo $(EQT)C_libdir :=$(libdir)$(EQT) >>conf.mk

-include conf.mk

ifneq ($(strip $(C_DEBUG))_$(strip $(C_GCC32))_$(strip $(C_libdir)),$(strip $(DEBUG))_$(strip $(GCC32))_$(strip $(libdir)))
.PHONY: conf.mk
endif

include src$(PSEP)mkd64.mk
include modules$(PSEP)modules.mk

bin: $(BINARIES)

modules: $(MODULES)

clean:
	$(RMF) $(SOURCES:.c=.o)
	$(RMF) $(BID)
	$(RMF) $(CLEAN)
	$(RMFR) $(OUT)
	$(RMFR) mkd64sdk

distclean: clean
	$(RMF) conf.mk
	$(RMF) $(SOURCES:.c=.d)

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

sdk: bin mkd64.a
	$(MDP) mkd64sdk$(PSEP)include$(PSEP)mkd64
	$(MDP) mkd64sdk$(PSEP)lib$(PSEP)mkd64
	$(MDP) mkd64sdk$(PSEP)examples$(PSEP)module
	$(CPF) include$(PSEP)mkd64$(PSEP)*.h mkd64sdk$(PSEP)include$(PSEP)mkd64
	-$(CPF) src/mkd64.a mkd64sdk$(PSEP)lib$(PSEP)mkd64
	$(CPF) examples$(PSEP)module$(PSEP)Makefile mkd64sdk$(PSEP)examples$(PSEP)module
	$(CPF) examples$(PSEP)module$(PSEP)module.c mkd64sdk$(PSEP)examples$(PSEP)module
	$(CPF) modapi.txt mkd64sdk
	$(CPF) coding.txt mkd64sdk

outdir:
	$(VR)$(MDP) $(OUT)

$(BID): tools$(PSEP)buildid.c $(SOURCES) Makefile conf.mk
	$(VCCLD)
	$(VR)$(CC) -o$@ $(mkd64_DEFINES) $(CFLAGS) $<

.PHONY: outdir all bin modules strip clean distclean install
.SUFFIXES:

# vim: noet:si:ts=8:sts=8:sw=8
