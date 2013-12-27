#ifndef MKD64_TRACK_H
#define MKD64_TRACK_H

#include <stdlib.h>
#include "block.h"

struct track;
typedef struct track Track;

Track *track_new(size_t num_sectors);
void track_delete(Track *this);

int track_readBlock(const Track *this, Block *block);
int track_writeBlock(Track *this, const Block *block);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
