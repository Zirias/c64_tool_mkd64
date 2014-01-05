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

ifdef DEBUG
CFLAGS += -DDEBUG -g3 -O0
else
CFLAGS += -g0 -O3
endif

ifdef prefix
BINDIR = /bin
LIBDIR = /lib
INSTALL = install
CFLAGS += -DMODDIR="\"$(prefix)$(LIBDIR)/mkd64\""
endif

ifdef GCC32
CC = gcc -m32
CFLAGS += -DGCC32BIT
else
CC = gcc
endif

INCLUDES = -Iinclude

mkd64_OBJS = mkd64.o image.o track.o block.o filemap.o diskfile.o \
	     cmdline.o modrepo.o random.o
mkd64_LDFLAGS = $(dl_LDFLAGS)
mkd64_DEFINES = -DBUILDING_MKD64

MODULES = cbmdos$(SO) dlphndos$(SO)

cbmdos_OBJS = modules$(PSEP)cbmdos.o

dlphndos_OBJS = modules$(PSEP)dlphndos.o

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

distclean: clean

strip:	all
	strip --strip-all mkd64$(EXE)
	strip --strip-unneeded *$(SO)

ifdef prefix

install: strip
	$(INSTALL) -d $(DESTDIR)$(prefix)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(prefix)$(LIBDIR)/mkd64
	$(INSTALL) mkd64$(EXE) $(DESTDIR)$(prefix)$(BINDIR)
	$(INSTALL) *$(SO) $(DESTDIR)$(prefix)$(LIBDIR)/mkd64

endif

mkd64$(EXE):	buildid.h $(mkd64_OBJS)
	$(CC) -o$@ $^ $(mkd64_LDFLAGS)

buildid$(EXE):	buildid.c
	$(CC) -o$@ $(mkd64_DEFINES) $(CFLAGS) buildid.c

buildid.h:	buildid$(EXE)
	.$(PSEP)buildid$(EXE) > buildid.h

modules$(PSEP)buildid.h:	buildid$(EXE)
	.$(PSEP)buildid$(EXE) > modules$(PSEP)buildid.h

mkd64.a:	$(mkd64_OBJS)
	dlltool -l$@ -Dmkd64.exe $^

modules$(PSEP)%.o:	modules$(PSEP)%.c modules$(PSEP)buildid.h
	$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

%.o:	%.c buildid.h
	$(CC) -o$@ -c $(mkd64_DEFINES) $(CFLAGS) $(INCLUDES) $<

cbmdos$(SO): $(cbmdos_OBJS) $(mod_LIBS)
	$(CC) -shared -o$@ $^

dlphndos$(SO): $(dlphndos_OBJS) $(mod_LIBS)
	$(CC) -shared -o$@ $^

.PHONY: all bin modules strip clean distclean install

.SUFFIXES:

# vim: noet:si:ts=8:sts=8:sw=8
