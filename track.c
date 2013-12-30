
#include <stdlib.h>
#include "stdintrp.h"
#include <string.h>

#include "track.h"
#include "block.h"
#include "blckstat.h"

#define TRACK_SIZE(x) (sizeof(Track) + (x - 1) * sizeof(Block *))

struct track
{
    size_t num_sectors;
    Block *sectors[1];
};

Track *
track_new(size_t num_sectors)
{
    int i;
    Track *this = malloc(TRACK_SIZE(num_sectors));
    this->num_sectors = num_sectors;
    for (i = 0; i < num_sectors; ++i)
    {
        this->sectors[i] = block_new();
    }
    return this;
}

void
track_delete(Track *this)
{
    int i;
    for (i = 0; i < this->num_sectors; ++i)
    {
        block_delete(this->sectors[i]);
    }
    free(this);
}

BlockStatus
track_blockStatus(const Track *this, int sector)
{
    if (sector < 1 || sector > this->num_sectors) return (BlockStatus) -1;
    return block_status(this->sectors[sector-1]);
}

int
track_reserveBlock(Track *this, int sector)
{
    if (sector < 1 || sector > this->num_sectors) return 0;
    return block_reserve(this->sectors[sector-1]);
}

int
track_allocateBlock(Track *this, int sector)
{
    if (sector < 1 || sector > this->num_sectors) return 0;
    return block_allocate(this->sectors[sector-1]);
}

Block *
track_block(Track *this, int sector)
{
    if (sector < 1 || sector > this->num_sectors) return 0;
    return this->sectors[sector-1];
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
