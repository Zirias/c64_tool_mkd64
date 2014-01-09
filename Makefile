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

dl_LDFLAGS = -ldl -Wl,-E
mod_CFLAGS = -fPIC
mod_LIBS =

endif

CFLAGS += -Werror=declaration-after-statement -fvisibility=hidden

ifdef DEBUG
CFLAGS += -DDEBUG -g3 -O0
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
endif

ifdef GCC32
CC = gcc -m32
CFLAGS += -DGCC32BIT
else
CC = gcc
endif

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
	$(CC) -o$@ $^ $(mkd64_LDFLAGS) $(LDFLAGS)

buildid$(EXE):	buildid.c
	$(CC) -o$@ $(mkd64_DEFINES) $(CFLAGS) buildid.c

buildid.h:	buildid$(EXE)
	.$(PSEP)buildid$(EXE) > buildid.h

modules$(PSEP)buildid.h:	buildid$(EXE)
	.$(PSEP)buildid$(EXE) > modules$(PSEP)buildid.h

mkd64.a:	$(mkd64_OBJS)
	-dlltool -l$@ -Dmkd64.exe $^

modules$(PSEP)%.o:	modules$(PSEP)%.c modules$(PSEP)buildid.h
	$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

%.o:	%.c buildid.h
	$(CC) -o$@ -c $(mkd64_DEFINES) $(CFLAGS) $(INCLUDES) $<

cbmdos$(SO): $(cbmdos_OBJS) $(mod_LIBS)
	$(CC) -shared -o$@ $^ $(LDFLAGS)

xtracks$(SO): $(xtracks_OBJS) $(mod_LIBS)
	$(CC) -shared -o$@ $^ $(LDFLAGS)

.PHONY: all bin modules strip clean distclean install

.SUFFIXES:

# vim: noet:si:ts=8:sts=8:sw=8
