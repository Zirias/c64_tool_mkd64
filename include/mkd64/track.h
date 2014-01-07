#ifndef MKD64_TRACK_H
#define MKD64_TRACK_H

#include <stdlib.h>

struct track;
typedef struct track Track;

#include <mkd64/block.h>
#include <mkd64/imodule.h>

DECLEXPORT Track *track_new(int tracknum, size_t num_sectors);
DECLEXPORT void track_delete(Track *this);

DECLEXPORT BlockStatus track_blockStatus(const Track *this, int sector);
DECLEXPORT size_t track_numSectors(const Track *this);
DECLEXPORT int track_freeSectors(const Track *this);
DECLEXPORT int track_freeSectorsRaw(const Track *this);

DECLEXPORT int track_reserveBlock(Track *this, int sector, IModule *by);
DECLEXPORT int track_allocateBlock(Track *this, int sector);
DECLEXPORT int track_allocateFirstFreeFrom(Track *this,
        int sector, int considerReserved);

DECLEXPORT Block *track_block(Track *this, int sector);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
