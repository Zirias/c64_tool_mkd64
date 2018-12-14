mkd64_MODULES:= mkd64 image track block defalloc filemap diskfile cmdline \
	modrepo util
mkd64_PLATFORMMODULES:= platformutil
mkd64_DEFINES:= -DBUILDING_MKD64
mkd64_win32_LIBS:= shlwapi
ifneq ($(plugindir),)
mkd64_CFLAGS:= -DMODDIR="\"$(plugindir)\""
endif
mkd64_posix_LDFLAGS:= -Wl,-E
ifneq ($(findstring linux,$(TARGETARCH)),)
mkd64_posix_LIBS:= dl
endif
$(call binrules, mkd64)

