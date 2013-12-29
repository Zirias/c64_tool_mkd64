#ifndef MKD64_MODREPO_H
#define MKD64_MODREPO_H

#include "imodule.h"

struct modrepo;
typedef struct modrepo Modrepo;

Modrepo *modrepo_new(const char *exe);
void modrepo_delete(Modrepo *this);

IModule *modrepo_moduleInstance(Modrepo *this, const char *id);
void modrepo_deleteInstance(Modrepo *this, IModule *instance);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
