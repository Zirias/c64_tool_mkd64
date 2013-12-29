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

CFLAGS += -DWIN32=1

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

endif

CC = gcc

mkd64_CLASSES = image.o track.o block.o diskfile.o cmdline.o modrepo.o

mkd64_OBJS = mkd64.o $(mkd64_CLASSES)
mkd64_LDFLAGS = -ldl

cmdtest_OBJS = cmdtest.o $(mkd64_CLASSES)
cmdtest_LDFLAGS = -ldl

MODULES = cbmdos$(SO)

cbmdos_OBJS = modules$(PSEP)cbmdos.o

all:	cmdtest$(EXE)

modules:	$(MODULES)

clean:
	$(RMF) *.o
	$(RMF) *.so
	$(RMF) modules$(PSEP)*.o
	$(RMF) cmdtest$(EXE)
	$(RMF) mkd64$(EXE)

mkd64$(EXE):	$(mkd64_OBJS)
	$(CC) -o$@ $(mkd64_LDFLAGS) $^

cmdtest$(EXE):	$(cmdtest_OBJS)
	$(CC) -o$@ $(cmdtest_LDFLAGS) $^

modules$(PSEP)%.o:	modules$(PSEP)%.c
	$(CC) -o$@ -c -fPIC $(CFLAGS) $<

%.o:	%.c
	$(CC) -o$@ -c $(CFLAGS) $<

cbmdos$(SO): $(cbmdos_OBJS)
	$(CC) -shared -o$@ $^

.PHONY:	all modules clean

.SUFFIXES:

# vim: noet:si:ts=8:sts=8:sw=8
