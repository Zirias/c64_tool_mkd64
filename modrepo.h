#ifndef MKD64_MODREPO_H
#define MKD64_MODREPO_H

#include "imodule.h"
#include "image.h"
#include "diskfile.h"
#include "track.h"
#include "block.h"

struct modrepo;
typedef struct modrepo Modrepo;

Modrepo *modrepo_new(const char *exe);
void modrepo_delete(Modrepo *this);

IModule *modrepo_moduleInstance(Modrepo *this, const char *id);

int modrepo_createInstance(Modrepo *this, const char *id);
int modrepo_deleteInstance(Modrepo *this, const char *id);
int modrepo_isActive(Modrepo *this, const char *id);

char *modrepo_getHelp(Modrepo *this, const char *id);

void modrepo_allInitImage(Modrepo *this, Image *image);
void modrepo_allGlobalOption(Modrepo *this, char opt, const char *arg);
void modrepo_allFileOption(Modrepo *this,
        Diskfile *file, char opt, const char *arg);
Track *modrepo_firstGetTrack(Modrepo *this, int track);
void modrepo_allFileWritten(Modrepo *this, Diskfile *file);
void modrepo_allStatusChanged(Modrepo *this, BlockPosition *pos);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
