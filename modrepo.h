#ifndef MODREPO_H
#define MODREPO_H

#include <mkd64/modrepo.h>
#include <mkd64/image.h>
#include <mkd64/diskfile.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

typedef void (*ModInstanceCreated)(void *owner, IModule *instance);

Modrepo *modrepo_new(const char *exe, void *owner,
        ModInstanceCreated callback);

void modrepo_delete(Modrepo *this);

void modrepo_reloadModules(Modrepo *this);

int modrepo_createInstance(Modrepo *this, const char *id);
int modrepo_deleteInstance(Modrepo *this, const char *id);

char *modrepo_getHelp(Modrepo *this, const char *id);
char *modrepo_getVersionInfo(Modrepo *this, const char *id);

void modrepo_allInitImage(Modrepo *this, Image *image);
int modrepo_allGlobalOption(Modrepo *this, char opt, const char *arg);
int modrepo_allFileOption(Modrepo *this,
        Diskfile *file, char opt, const char *arg);
Track *modrepo_firstGetTrack(Modrepo *this, int track);
void modrepo_allFileWritten(Modrepo *this,
        Diskfile *file, const BlockPosition *start);
void modrepo_allStatusChanged(Modrepo *this, const BlockPosition *pos);
void modrepo_allImageComplete(Modrepo *this);

const char *modrepo_nextAvailableModule(Modrepo *this, const char *id);
const char *modrepo_nextLoadedModule(Modrepo *this, const char *id);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
