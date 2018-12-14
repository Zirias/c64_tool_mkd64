sepgen_MODULES:= module
sepgen_LIBTYPE:= plugin
sepgen_DEPS:= mkd64
sepgen_win32_LIBS:= mkd64
$(call librules, sepgen)

