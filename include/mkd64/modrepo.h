#ifndef MKD64_MODREPO_H
#define MKD64_MODREPO_H

#include <mkd64/imodule.h>

struct modrepo;
typedef struct modrepo Modrepo;

DECLEXPORT IModule *modrepo_moduleInstance(Modrepo *this, const char *id);

DECLEXPORT int modrepo_isActive(Modrepo *this, const char *id);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
