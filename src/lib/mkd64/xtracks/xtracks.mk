xtracks_MODULES:= module
xtracks_LIBTYPE:= plugin
xtracks_INSTALLDIRNAME:= plugin
xtracks_DEPS:= mkd64
xtracks_win32_LIBS:= mkd64
$(call librules, xtracks)

