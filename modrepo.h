#ifndef MKD64_MODREPO_H
#define MKD64_MODREPO_H

#include "imodule.h"

struct modrepo;
typedef struct modrepo Modrepo;

Modrepo *modrepo_new(const char *exe);
void modrepo_delete(Modrepo *this);

IModule *modrepo_moduleInstance(Modrepo *this, const char *id);

int modrepo_createInstance(Modrepo *this, const char *id);
int modrepo_deleteInstance(Modrepo *this, const char *id);
int modrepo_isActive(Modrepo *this, const char *id);

char *modrepo_getHelp(Modrepo *this, const char *id);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
