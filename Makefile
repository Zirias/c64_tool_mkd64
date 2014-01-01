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

endif

ifdef DEBUG
CFLAGS += -DDEBUG -g3 -O0
else
CFLAGS += -g0 -O3
endif

CC = gcc

mkd64_OBJS = mkd64.o image.o track.o block.o filemap.o diskfile.o \
	     cmdline.o modrepo.o random.o
mkd64_LDFLAGS = $(dl_LDFLAGS)

MODULES = cbmdos$(SO)

cbmdos_OBJS = modules$(PSEP)cbmdos.o

all:	bin modules

bin:	mkd64$(EXE)

modules:	$(MODULES)

clean:
	$(RMF) *.o
	$(RMF) *$(SO)
	$(RMF) modules$(PSEP)*.o
	$(RMF) cmdtest$(EXE)
	$(RMF) mkd64$(EXE)

strip:	all
	strip --strip-all mkd64
	strip --strip-unneeded *$(SO)

mkd64$(EXE):	$(mkd64_OBJS)
	$(CC) -o$@ $^ $(mkd64_LDFLAGS)

modules$(PSEP)%.o:	modules$(PSEP)%.c
	$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $<

%.o:	%.c
	$(CC) -o$@ -c $(CFLAGS) $<

cbmdos$(SO): $(cbmdos_OBJS)
	$(CC) -shared -o$@ $^

.PHONY:	all bin modules strip clean

.SUFFIXES:

# vim: noet:si:ts=8:sts=8:sw=8
