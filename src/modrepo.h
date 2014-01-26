#ifndef MODREPO_H
#define MODREPO_H

#include <stdlib.h>

#include <mkd64/modrepo.h>
#include <mkd64/image.h>
#include <mkd64/diskfile.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

typedef struct ModInstIterator ModInstIterator;

typedef void (*ModInstanceCreated)(void *owner, IModule *instance);

size_t ModRepo_objectSize(void);

ModRepo *ModRepo_init(ModRepo *self, void *owner, ModInstanceCreated callback);

void ModRepo_done(ModRepo *self);

IModule *ModRepo_createInstance(ModRepo *self, const char *id);

char *ModRepo_getHelp(const ModRepo *self, const char *id);
char *ModRepo_getVersionInfo(const ModRepo *self, const char *id);

void ModRepo_allInitImage(const ModRepo *self, Image *image);
int ModRepo_allGlobalOption(const ModRepo *self, char opt, const char *arg);
int ModRepo_allFileOption(const ModRepo *self,
        DiskFile *file, char opt, const char *arg);
Track *ModRepo_firstGetTrack(const ModRepo *self, int track);
void ModRepo_allFileWritten(const ModRepo *self,
        DiskFile *file, const BlockPosition *start);
void ModRepo_allStatusChanged(const ModRepo *self, const BlockPosition *pos);
void ModRepo_allImageComplete(const ModRepo *self);

const char *ModRepo_nextAvailableModule(const ModRepo *self, const char *id);
const char *ModRepo_nextLoadedModule(const ModRepo *self, const char *id);

ModInstIterator *ModRepo_createIterator(const ModRepo *self);
int ModInstIterator_moveNext(ModInstIterator *self);
IModule *ModInstIterator_current(const ModInstIterator *self);
void ModInstIterator_free(ModInstIterator *self);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
