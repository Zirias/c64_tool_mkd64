cbmdos_MODULES:= module alloc
cbmdos_LIBTYPE:= plugin
cbmdos_INSTALLDIRNAME:= plugin
cbmdos_DEPS:= mkd64
cbmdos_win32_LIBS:= mkd64
$(call librules, cbmdos)

