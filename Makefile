ifeq ($(OS),Windows_NT)

EXE = .exe
SO = .dll
CMDSEP = &
PSEP = \\
CPF = copy /y
RMF = del /f /q
RMFR = -rd /s /q
MDP = -md
XIF = if exist
XTHEN = (
XFI = )
CATIN = copy /b
CATADD = +
CATOUT =
EQ=

CFLAGS += -DWIN32
dl_LDFLAGS = -lshlwapi 
mod_CFLAGS =
mod_LIBS = mkd64.a

else

EXE =
SO = .so
CMDSEP = ;
PSEP = /
CPF = cp -f
RMF = rm -f
RMFR = rm -fr
MDP = mkdir -p
XIF = if [ -x
XTHEN = ]; then
XFI = ; fi
CATIN = cat
CATADD = 
CATOUT = >
EQ="
#" make vim syntax highlight happy

dl_LDFLAGS = -ldl -Wl,-E
mod_CFLAGS = -fPIC
mod_LIBS =

endif

CFLAGS += -Wall -Werror=implicit-int -Werror=implicit-function-declaration -Werror=declaration-after-statement -fvisibility=hidden

VTAGS =
V=0

ifdef DEBUG
CFLAGS += -DDEBUG -g3 -O0
VTAGS += [DBG]
else
CFLAGS += -g0 -O3 -flto
LDFLAGS += -flto
endif

ifdef prefix
bindir = $(prefix)/bin
libbasedir = $(prefix)/lib
libdir = $(libbasedir)/mkd64
includebasedir = $(prefix)/include
includedir = $(includebasedir)/mkd64
docbasedir = $(prefix)/share/doc
docdir = $(docbasedir)/mkd64

INSTALL = install
CFLAGS += -DMODDIR="\"$(libdir)\""
else
VTAGS += [PRT]
endif

ifdef GCC32
CC = gcc -m32
CFLAGS += -DGCC32BIT
VTAGS += [32]
else
CC = gcc
endif

cc__v_0 = @echo $(EQ)  $(VTAGS)   [CC]   $@$(EQ)
cc__v_1 =
ld__v_0 = @echo $(EQ)  $(VTAGS)   [LD]   $@$(EQ)
ld__v_1 =
ccld__v_0 = @echo $(EQ)  $(VTAGS)   [CCLD] $@$(EQ)
ccld__v_1 =
gen__v_0 = @echo $(EQ)  $(VTAGS)   [GEN]  $@$(EQ)
gen__v_1 =
r__v_0 = @
r__v_1 =

VCC = $(cc__v_$(V))
VLD = $(ld__v_$(V))
VCCLD = $(ccld__v_$(V))
VGEN = $(gen__v_$(V))
VR = $(r__v_$(V))

INCLUDES = -Iinclude

mkd64_OBJS = mkd64.o image.o track.o block.o filemap.o diskfile.o \
	     cmdline.o modrepo.o util.o
mkd64_LDFLAGS = $(dl_LDFLAGS)
mkd64_DEFINES = -DBUILDING_MKD64

MODULES = cbmdos$(SO) xtracks$(SO)

cbmdos_OBJS = modules$(PSEP)cbmdos.o

xtracks_OBJS = modules$(PSEP)xtracks.o

all:	bin modules

bin:	mkd64$(EXE)

modules:	$(MODULES)

clean:
	$(RMF) *.o
	$(RMF) *.a
	$(RMF) *$(SO)
	$(RMF) modules$(PSEP)*.o
	$(RMF) mkd64$(EXE)
	$(RMF) buildid$(EXE)
	$(RMF) buildid.h
	$(RMF) modules$(PSEP)buildid.h
	$(RMFR) mkd64sdk

distclean: clean

strip:	all
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

sdk:	bin mkd64.a
	$(MDP) mkd64sdk$(PSEP)include$(PSEP)mkd64
	$(MDP) mkd64sdk$(PSEP)lib$(PSEP)mkd64
	$(MDP) mkd64sdk$(PSEP)examples$(PSEP)module
	$(CPF) include$(PSEP)mkd64$(PSEP)*.h mkd64sdk$(PSEP)include$(PSEP)mkd64
	-$(CPF) mkd64.a mkd64sdk$(PSEP)lib$(PSEP)mkd64
	$(CPF) examples$(PSEP)module$(PSEP)Makefile mkd64sdk$(PSEP)examples$(PSEP)module
	$(CPF) examples$(PSEP)module$(PSEP)module.c mkd64sdk$(PSEP)examples$(PSEP)module
	$(CPF) modapi.txt mkd64sdk

mkd64$(EXE):	buildid.h $(mkd64_OBJS)
	$(VLD)
	$(VR)$(CC) -o$@ $^ $(mkd64_LDFLAGS) $(LDFLAGS)

buildid$(EXE):	buildid.c
	$(VCCLD)
	$(VR)$(CC) -o$@ $(mkd64_DEFINES) $(CFLAGS) buildid.c

buildid.h:	buildid$(EXE)
	$(VGEN)
	$(VR).$(PSEP)buildid$(EXE) > buildid.h

modules$(PSEP)buildid.h:	buildid$(EXE)
	$(VGEN)
	$(VR).$(PSEP)buildid$(EXE) > modules$(PSEP)buildid.h

mkd64.a:	$(mkd64_OBJS)
	$(VGEN)
	$(VR)-dlltool -l$@ -Dmkd64.exe $^

modules$(PSEP)%.o:	modules$(PSEP)%.c modules$(PSEP)buildid.h
	$(VCC)
	$(VR)$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

%.o:	%.c buildid.h
	$(VCC)
	$(VR)$(CC) -o$@ -c $(mkd64_DEFINES) $(CFLAGS) $(INCLUDES) $<

cbmdos$(SO): $(cbmdos_OBJS) $(mod_LIBS)
	$(VLD)
	$(VR)$(CC) -shared -o$@ $^ $(LDFLAGS)

xtracks$(SO): $(xtracks_OBJS) $(mod_LIBS)
	$(VLD)
	$(VR)$(CC) -shared -o$@ $^ $(LDFLAGS)

.PHONY: all bin modules strip clean distclean install

.SUFFIXES:

# vim: noet:si:ts=8:sts=8:sw=8
