#ifndef MKD64_TRACK_H
#define MKD64_TRACK_H

#include <stdlib.h>

#include "block.h"
#include "blckstat.h"

struct track;
typedef struct track Track;

Track *track_new(size_t num_sectors);
void track_delete(Track *this);

BlockStatus track_blockStatus(const Track *this, int sector);
int track_reserveBlock(Track *this, int sector);
int track_allocateBlock(Track *this, int sector);

Block *track_block(Track *this, int sector);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
