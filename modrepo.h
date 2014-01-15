#ifndef MODREPO_H
#define MODREPO_H

#include <stdlib.h>

#include <mkd64/modrepo.h>
#include <mkd64/image.h>
#include <mkd64/diskfile.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

typedef void (*ModInstanceCreated)(void *owner, IModule *instance);

size_t ModRepo_objectSize(void);

ModRepo *ModRepo_init(ModRepo *this, const char *exe, void *owner,
        ModInstanceCreated callback);

void ModRepo_done(ModRepo *this);

void ModRepo_reloadModules(ModRepo *this);

int ModRepo_createInstance(ModRepo *this, const char *id);
int ModRepo_deleteInstance(ModRepo *this, const char *id);

char *ModRepo_getHelp(const ModRepo *this, const char *id);
char *ModRepo_getVersionInfo(const ModRepo *this, const char *id);

void ModRepo_allInitImage(const ModRepo *this, Image *image);
int ModRepo_allGlobalOption(const ModRepo *this, char opt, const char *arg);
int ModRepo_allFileOption(const ModRepo *this,
        DiskFile *file, char opt, const char *arg);
Track *ModRepo_firstGetTrack(const ModRepo *this, int track);
void ModRepo_allFileWritten(const ModRepo *this,
        DiskFile *file, const BlockPosition *start);
void ModRepo_allStatusChanged(const ModRepo *this, const BlockPosition *pos);
void ModRepo_allImageComplete(const ModRepo *this);

const char *ModRepo_nextAvailableModule(const ModRepo *this, const char *id);
const char *ModRepo_nextLoadedModule(const ModRepo *this, const char *id);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
