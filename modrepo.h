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

char *modrepo_getHelp(const Modrepo *this, const char *id);
char *modrepo_getVersionInfo(const Modrepo *this, const char *id);

void modrepo_allInitImage(const Modrepo *this, Image *image);
int modrepo_allGlobalOption(const Modrepo *this, char opt, const char *arg);
int modrepo_allFileOption(const Modrepo *this,
        Diskfile *file, char opt, const char *arg);
Track *modrepo_firstGetTrack(const Modrepo *this, int track);
void modrepo_allFileWritten(const Modrepo *this,
        Diskfile *file, const BlockPosition *start);
void modrepo_allStatusChanged(const Modrepo *this, const BlockPosition *pos);
void modrepo_allImageComplete(const Modrepo *this);

const char *modrepo_nextAvailableModule(const Modrepo *this, const char *id);
const char *modrepo_nextLoadedModule(const Modrepo *this, const char *id);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
