#ifndef MKD64_IMODULE_H
#define MKD64_IMODULE_H

#include "image.h"
#include "diskfile.h"
#include "track.h"
#include "block.h"

struct iModule;
typedef struct iModule IModule;

struct iModule
{
    const char *(*id)(void);

    void (*initImage)(IModule *this, Image *image);

    void (*globalOption)(IModule *this, char opt, const char *arg);

    void (*fileOption)(IModule *this,
            Diskfile *file, char opt, const char *arg);

    Track *(*getTrack)(IModule *this, int track);

    void (*fileWritten)(IModule *this,
            Diskfile *file, const BlockPosition *start);

    void (*statusChanged)(IModule *this, const BlockPosition *pos);
};

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
