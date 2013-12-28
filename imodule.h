#ifndef MKD64_IMODULE_H
#define MKD64_IMODULE_H

#include "image.h"
#include "diskfile.h"
#include "block.h"

struct iModule;
typedef struct iModule IModule;

struct iModule
{
    const char *(*id)(void);
    void (*init)(IModule *this, Image *image);
    void (*fileWritten)(IModule *this, Diskfile *file);
    void (*statusChanged)(IModule *this, BlockPosition *pos);
};

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
