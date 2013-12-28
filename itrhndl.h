#ifndef MKD64_ITRHNDL_H
#define MKD64_ITRHNDL_H

#include "diskfile.h"
#include "blckstat.h"

struct iTrackHandler;
typedef struct iTrackHandler ITrackHandler;

struct iTrackHandler
{
    void (*init)(ITrackHandler *this, Track *track);
    void (*fileWritten)(ITrackHandler *this, Diskfile *file);
    void (*statusChanged)(ITrackHandler *this, int sector, BlockStatus status);
};

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
