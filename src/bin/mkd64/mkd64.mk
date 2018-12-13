mkd64_MODULES:= mkd64 image track block defalloc filemap diskfile cmdline \
	modrepo util
mkd64_PLATFORMMODULES:= platformutil
mkd64_DEFINES:= -DBUILDING_MKD64
mkd64_win32_LIBS:= shlwapi
ifeq ($(filter-out 0 false FALSE no NO,$(PORTABLE)),)
mkd64_CFLAGS:= -DMODDIR="\"$(libdir)/mkd64\""
endif
mkd64_posix_LDFLAGS:= -Wl,-E
ifneq ($(findstring linux,$(TARGETARCH)),)
mkd64_posix_LIBS:= dl
endif
$(call binrules, mkd64)

