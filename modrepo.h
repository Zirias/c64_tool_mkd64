#ifndef MODREPO_H
#define MODREPO_H

#include <mkd64/modrepo.h>
#include <mkd64/image.h>
#include <mkd64/diskfile.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

Modrepo *modrepo_new(const char *exe);
void modrepo_delete(Modrepo *this);

int modrepo_createInstance(Modrepo *this, const char *id);
int modrepo_deleteInstance(Modrepo *this, const char *id);

char *modrepo_getHelp(Modrepo *this, const char *id);

void modrepo_allInitImage(Modrepo *this, Image *image);
void modrepo_allGlobalOption(Modrepo *this, char opt, const char *arg);
void modrepo_allFileOption(Modrepo *this,
        Diskfile *file, char opt, const char *arg);
Track *modrepo_firstGetTrack(Modrepo *this, int track);
void modrepo_allFileWritten(Modrepo *this,
        Diskfile *file, const BlockPosition *start);
void modrepo_allStatusChanged(Modrepo *this, const BlockPosition *pos);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
