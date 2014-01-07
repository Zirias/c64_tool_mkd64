#include <mkd64/common.h>

#include <stdlib.h>
#include <string.h>

#include <mkd64/track.h>
#include "block.h"
#include "modrepo.h"
#include "mkd64.h"

#define TRACK_SIZE(x) (sizeof(Track) + (x - 1) * sizeof(Block *))

struct track
{
    int tracknum;
    size_t num_sectors;
    int free_sectors;
    Block *sectors[1];
};

static void
blockStatusChanged(void *owner, Block *block,
        BlockStatus oldStatus, BlockStatus newStatus)
{
    Track *this = (Track *) owner;
    Modrepo *mr = mkd64_modrepo();

    if (oldStatus == BS_NONE && newStatus != BS_NONE)
        --(this->free_sectors);
    else if (oldStatus != BS_NONE && newStatus == BS_NONE)
        ++(this->free_sectors);

    modrepo_allStatusChanged(mr, block_position(block));
}

SOEXPORT Track *
track_new(int tracknum, size_t num_sectors)
{
    int i;
    BlockPosition pos;

    Track *this = malloc(TRACK_SIZE(num_sectors));
    this->tracknum = tracknum;
    this->num_sectors = num_sectors;
    this->free_sectors = num_sectors;

    pos.track = tracknum;
    for (i = 0; i < num_sectors; ++i)
    {
        pos.sector = i;
        this->sectors[i] = block_new(this, &pos, &blockStatusChanged);
    }
    return this;
}

SOEXPORT void
track_delete(Track *this)
{
    int i;
    for (i = 0; i < this->num_sectors; ++i)
    {
        block_delete(this->sectors[i]);
    }
    free(this);
}

SOEXPORT BlockStatus
track_blockStatus(const Track *this, int sector)
{
    if (sector < 0 || sector >= this->num_sectors) return (BlockStatus) -1;
    return block_status(this->sectors[sector]);
}

SOEXPORT size_t
track_numSectors(const Track *this)
{
    return this->num_sectors;
}

SOEXPORT int
track_freeSectors(const Track *this)
{
    return this->free_sectors;
}

SOEXPORT int
track_freeSectorsRaw(const Track *this)
{
    int free = 0;
    int i;

    for (i = 0; i < this->num_sectors; ++i)
    {
        if (!(block_status(this->sectors[i]) & BS_ALLOCATED)) ++free;
    }

    return free;
}

SOEXPORT int
track_reserveBlock(Track *this, int sector, IModule *by)
{
    if (sector < 0 || sector >= this->num_sectors) return 0;
    return block_reserve(this->sectors[sector], by);
}

SOEXPORT int
track_allocateBlock(Track *this, int sector)
{
    if (sector < 0 || sector >= this->num_sectors) return 0;
    return block_allocate(this->sectors[sector]);
}

SOEXPORT int
track_allocateFirstFreeFrom(Track *this, int sector, int askModules)
{
    Block *b;
    BlockStatus s;
    int i;

    if (sector < 0) return -1;

    for (i = this->num_sectors; i > 0 ; --i, ++sector)
    {
        if (sector >= this->num_sectors) sector = 0;
        b = this->sectors[sector];
        s = block_status(b);

        if (s == BS_NONE)
        {
            block_allocate(b);
            return sector;
        }
        if (askModules && s == BS_RESERVED && block_unReserve(b))
        {
            block_allocate(b);
            return sector;
        }
    }

    return -1;
}

SOEXPORT Block *
track_block(Track *this, int sector)
{
    if (sector < 0 || sector >= this->num_sectors) return 0;
    return this->sectors[sector];
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
