# example makefile for an mkd64 module
#
# This makefile needs GNU make.
#
# build module with 'make MKD64=/path/to/mkd64sdk' or adjust MKD64 variable
# here:

MKD64=/usr/include

# the name of the module shared object:
MODULE=example

ifeq ($(OS),Windows_NT)

SO = .dll
PSEP = \\
RMF = del /f /q

CFLAGS += -DWIN32
mod_CFLAGS =
mod_LIBS = $(MKD64)\\lib\\mkd64\\mkd64.a

else

SO = .so
PSEP = /
RMF = rm -f
XIF = if [ -x
XTHEN = ]; then
XFI = ; fi

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

INCLUDES = -I$(MKD64)

OUTPUT = $(MODULE)$(SO)

OBJS = module.o

all:	$(OUTPUT)

clean:
	$(RMF) *.o
	$(RMF) $(OUTPUT)

distclean: clean

strip:	all
	strip --strip-unneeded *$(SO)

%.o:	%.c
	$(CC) -o$@ -c $(mod_CFLAGS) $(CFLAGS) $(INCLUDES) $<

$(OUTPUT): $(OBJS) $(mod_LIBS)
	$(CC) -shared -o$@ $^ $(LDFLAGS)

.PHONY: all strip clean distclean

.SUFFIXES:

# vim: noet:si:ts=8:sts=8:sw=8
