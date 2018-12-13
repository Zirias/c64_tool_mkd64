include zimk/zimk.mk

INCLUDES += -I.$(PSEP)include

$(call zinc, src/bin/buildid/buildid.mk)
$(call zinc, src/bin/mkd64/mkd64.mk)
$(call zinc, src/lib/mkd64/cbmdos/cbmdos.mk)
$(call zinc, src/lib/mkd64/sepgen/sepgen.mk)
$(call zinc, src/lib/mkd64/xtracks/xtracks.mk)

$(mkd64_OBJDIR)$(PSEP)mkd64.d $(cbmdos_OBJDIR)$(PSEP)module.d \
	$(sepgen_OBJDIR)$(PSEP)module.d $(xtracks_OBJDIR)$(PSEP)module.d: \
	include$(PSEP)buildid.h

include$(PSEP)buildid.h: $(buildid_EXE)
	$(buildid_EXE) >$@

clean::
	$(RMF) include$(PSEP)buildid.h

