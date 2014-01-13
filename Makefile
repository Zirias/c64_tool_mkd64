ifeq ($(OS),Windows_NT)

EXE := .exe
SO := .dll
CMDSEP := &
PSEP := \\
CPF := copy /y
RMF := del /f /q
RMFR := -rd /s /q
MDP := -md
XIF := if exist
XTHEN := (
XFI := )
CATIN := copy /b
CATADD := +
CATOUT :=
EQT :=

CFLAGS += -DWIN32
dl_LDFLAGS := -lshlwapi 
mod_CFLAGS :=
mod_LIBS := mkd64.a

else

EXE :=
SO := .so
CMDSEP := ;
PSEP := /
CPF := cp -f
RMF := rm -f
RMFR := rm -fr
MDP := mkdir -p
XIF := if [ -x
XTHEN := ]; then
XFI := ; fi
CATIN := cat
CATADD := 
CATOUT := >
EQT := "
#" make vim syntax highlight happy

dl_LDFLAGS := -ldl -Wl,-E
mod_CFLAGS := -fPIC
mod_LIBS :=

endif

CFLAGS += -fvisibility=hidden -std=c99 -Wall -pedantic \
	  -Werror=implicit-int \
	  -Werror=implicit-function-declaration \
	  -Werror=declaration-after-statement

LDFLAGS += -static-libgcc

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

INCLUDES := -Iinclude

mkd64_OBJS := mkd64.o image.o track.o block.o defalloc.o filemap.o diskfile.o \
	     cmdline.o modrepo.o util.o
mkd64_LDFLAGS := $(dl_LDFLAGS)
mkd64_DEFINES := -DBUILDING_MKD64

MODULES := cbmdos$(SO) xtracks$(SO) sepgen$(SO)

cbmdos_OBJS := modules$(PSEP)cbmdos.o modules$(PSEP)cbmdos$(PSEP)alloc.o

xtracks_OBJS := modules$(PSEP)xtracks.o

sepgen_OBJS := modules$(PSEP)sepgen.o

mkd64_SOURCES := $(mkd64_OBJS:.o=.c)
mod_SOURCES := $(cbmdos_OBJS:.o=.c) $(xtracks_OBJS:.o=.c) $(sepgen_OBJS:.o=.c)

all: bin modules

bin: mkd64$(EXE)

modules: $(MODULES)

clean:
	$(RMF) *.o
	$(RMF) *.a
	$(RMF) *$(SO)
	$(RMF) modules$(PSEP)*.o
	$(RMF) modules$(PSEP)cbmdos$(PSEP)*.o
	$(RMF) mkd64$(EXE)
	$(RMF) buildid$(EXE)
	$(RMF) buildid.h
	$(RMF) modules$(PSEP)buildid.h
	$(RMFR) mkd64sdk

distclean: clean
	$(RMF) conf.mk
	$(RMF) *.d
	$(RMF) modules$(PSEP)*.d
	$(RMF) modules$(PSEP)cbmdos$(PSEP)*.d

strip: all
	strip --strip-all mkd64$(EXE)
	strip --strip-unneeded *$(SO)

ifdef prefix

install: strip
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -d $(DESTDIR)$(libdir)
	$(INSTALL) -d $(DESTDIR)$(includedir)
	$(INSTALL) -d $(DESTDIR)$(docdir)/examples/module
	$(INSTALL) mkd64 $(DESTDIR)$(bindir)
	$(INSTALL) *.so $(DESTDIR)$(libdir)
	$(INSTALL) -m644 include/mkd64/*.h $(DESTDIR)$(includedir)
	$(INSTALL) -m644 README.md $(DESTDIR)$(docdir)
	$(INSTALL) -m644 modapi.txt $(DESTDIR)$(docdir)
	$(INSTALL) -m644 examples/module/* $(DESTDIR)$(docdir)/examples/module

endif

sdk: bin mkd64.a
	$(MDP) mkd64sdk$(PSEP)include$(PSEP)mkd64
	$(MDP) mkd64sdk$(PSEP)lib$(PSEP)mkd64
	$(MDP) mkd64sdk$(PSEP)examples$(PSEP)module
	$(CPF) include$(PSEP)mkd64$(PSEP)*.h mkd64sdk$(PSEP)include$(PSEP)mkd64
	-$(CPF) mkd64.a mkd64sdk$(PSEP)lib$(PSEP)mkd64
	$(CPF) examples$(PSEP)module$(PSEP)Makefile mkd64sdk$(PSEP)examples$(PSEP)module
	$(CPF) examples$(PSEP)module$(PSEP)module.c mkd64sdk$(PSEP)examples$(PSEP)module
	$(CPF) modapi.txt mkd64sdk

conf.mk:
	$(VGENT)
	$(VR)echo $(EQT)C_DEBUG :=$(DEBUG)$(EQT) >conf.mk
	$(VR)echo $(EQT)C_GCC32 :=$(GCC32)$(EQT) >>conf.mk
	$(VR)echo $(EQT)C_libdir :=$(libdir)$(EQT) >>conf.mk

-include conf.mk

ifneq ($(strip $(C_DEBUG))_$(strip $(C_GCC32))_$(strip $(C_libdir)),$(strip $(DEBUG))_$(strip $(GCC32))_$(strip $(libdir)))
.PHONY: conf.mk
endif

buildid$(EXE): buildid.c $(mkd64_SOURCES) $(mod_SOURCES) Makefile conf.mk
	$(VCCLD)
	$(VR)$(CC) -o$@ $(mkd64_DEFINES) $(CFLAGS) buildid.c

buildid.h: $(mkd64_SOURCES) Makefile conf.mk | buildid$(EXE)
	$(VGEN)
	$(VR).$(PSEP)buildid$(EXE) > buildid.h

modules$(PSEP)buildid.h: $(mod_SOURCES) Makefile conf.mk | buildid$(EXE)
	$(VGEN)
	$(VR).$(PSEP)buildid$(EXE) > modules$(PSEP)buildid.h

mkd64$(EXE): $(mkd64_OBJS)
	$(VLD)
	$(VR)$(CC) -o$@ $^ $(mkd64_LDFLAGS) $(LDFLAGS)

mkd64.a: $(mkd64_OBJS)
	$(VGEN)
	$(VR)-dlltool -l$@ -Dmkd64.exe $^

cbmdos$(SO): $(cbmdos_OBJS) $(mod_LIBS)
	$(VLD)
	$(VR)$(CC) -shared -o$@ $^ $(LDFLAGS)

xtracks$(SO): $(xtracks_OBJS) $(mod_LIBS)
	$(VLD)
	$(VR)$(CC) -shared -o$@ $^ $(LDFLAGS)

sepgen$(SO): $(sepgen_OBJS) $(mod_LIBS)
	$(VLD)
	$(VR)$(CC) -shared -o$@ $^ $(LDFLAGS)

modules$(PSEP)%.d: modules$(PSEP)%.c Makefile conf.mk | modules$(PSEP)buildid.h
	$(VDEP)
	$(VR)$(CCDEP) -MT"$@ $(@:.d=.o)" -MF$@ \
		$(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

%.d: .$(PSEP)$(PSEP)%.c Makefile conf.mk | buildid.h
	$(VDEP)
	$(VR)$(CCDEP) -MT"$@ $(@:.d=.o)" -MF$@ \
		$(mkd64_CFLAGS) $(CFLAGS) $(INCLUDES) $<

-include $(cbmdos_OBJS:.o=.d)
-include $(xtracks_OBJS:.o=.d)
-include $(sepgen_OBJS:.o=.d)
modules$(PSEP)%.o: modules$(PSEP)%.c Makefile conf.mk
	$(VCC)
	$(VR)$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

-include $(mkd64_OBJS:.o=.d)
%.o: .$(PSEP)%.c Makefile conf.mk
	$(VCC)
	$(VR)$(CC) -o$@ -c $(mkd64_DEFINES) $(CFLAGS) $(INCLUDES) $<

.PHONY: all bin modules strip clean distclean install
.SUFFIXES:

# vim: noet:si:ts=8:sts=8:sw=8
