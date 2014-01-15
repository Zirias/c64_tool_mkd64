#include <mkd64/common.h>

#include <stdlib.h>
#include <string.h>

#include <mkd64/track.h>
#include <mkd64/debug.h>
#include "block.h"
#include "modrepo.h"
#include "mkd64.h"

struct Track
{
    int tracknum;
    size_t num_sectors;
    int free_sectors;
    Block *sectors[TRACK_MAX_SECTORS];
};

static void
blockStatusChanged(void *owner, const Block *block,
        BlockStatus oldStatus, BlockStatus newStatus)
{
    Track *this = (Track *) owner;
    ModRepo *mr = Mkd64_modRepo(MKD64);

    if (oldStatus == BS_NONE && newStatus != BS_NONE)
        --(this->free_sectors);
    else if (oldStatus != BS_NONE && newStatus == BS_NONE)
        ++(this->free_sectors);

    ModRepo_allStatusChanged(mr, Block_position(block));
}

SOEXPORT size_t
Track_objectSize(void)
{
    return sizeof(Track);
}

SOEXPORT Track *
Track_init(Track *this, int tracknum, size_t num_sectors)
{
    int i;
    BlockPosition pos;

    if (num_sectors > TRACK_MAX_SECTORS)
    {
        DBGd1("Error: Too many sectors requested", (int) num_sectors);
        num_sectors = TRACK_MAX_SECTORS;
    }

    this->tracknum = tracknum;
    this->num_sectors = num_sectors;
    this->free_sectors = num_sectors;

    pos.track = tracknum;
    for (i = 0; i < num_sectors; ++i)
    {
        pos.sector = i;
        this->sectors[i] = OBJNEW3(Block, this, &pos, &blockStatusChanged);
    }
    return this;
}

SOEXPORT void
Track_done(Track *this)
{
    int i;
    for (i = 0; i < this->num_sectors; ++i)
    {
        OBJDEL(Block, this->sectors[i]);
    }
}

SOEXPORT BlockStatus
Track_blockStatus(const Track *this, int sector)
{
    if (sector < 0 || sector >= this->num_sectors) return (BlockStatus) -1;
    return Block_status(this->sectors[sector]);
}

SOEXPORT size_t
Track_numSectors(const Track *this)
{
    return this->num_sectors;
}

static int
_freeSectorsRaw(const Track *this, BlockStatus mask)
{
    int free = 0;
    int i;

    for (i = 0; i < this->num_sectors; ++i)
    {
        if (!(Block_status(this->sectors[i]) & ~mask)) ++free;
    }

    return free;
}

SOEXPORT int
Track_freeSectors(const Track *this, BlockStatus mask)
{
    if (mask) return _freeSectorsRaw(this, mask);
    return this->free_sectors;
}

SOEXPORT int
Track_reserveBlock(Track *this, int sector, IModule *by)
{
    if (sector < 0 || sector >= this->num_sectors) return 0;
    return Block_reserve(this->sectors[sector], by);
}

SOEXPORT int
Track_allocateBlock(Track *this, int sector)
{
    if (sector < 0 || sector >= this->num_sectors) return 0;
    return Block_allocate(this->sectors[sector]);
}

SOEXPORT int
Track_allocateFirstFreeFrom(Track *this, int sector, int askModules)
{
    Block *b;
    BlockStatus s;
    int i;

    if (sector < 0) return -1;

    for (i = this->num_sectors; i > 0 ; --i, ++sector)
    {
        if (sector >= this->num_sectors) sector = 0;
        b = this->sectors[sector];
        s = Block_status(b);

        if (s == BS_NONE)
        {
            Block_allocate(b);
            return sector;
        }
        if (askModules && s == BS_RESERVED && Block_unReserve(b))
        {
            Block_allocate(b);
            return sector;
        }
    }

    return -1;
}

SOEXPORT Block *
Track_block(const Track *this, int sector)
{
    if (sector < 0 || sector >= this->num_sectors) return 0;
    return this->sectors[sector];
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
