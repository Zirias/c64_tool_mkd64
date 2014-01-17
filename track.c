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
    Track *self = (Track *) owner;
    ModRepo *mr = Mkd64_modRepo(MKD64);

    if (oldStatus == BS_NONE && newStatus != BS_NONE)
        --(self->free_sectors);
    else if (oldStatus != BS_NONE && newStatus == BS_NONE)
        ++(self->free_sectors);

    ModRepo_allStatusChanged(mr, Block_position(block));
}

SOEXPORT size_t
Track_objectSize(void)
{
    return sizeof(Track);
}

SOEXPORT Track *
Track_init(Track *self, int tracknum, size_t num_sectors)
{
    unsigned int i;
    BlockPosition pos;

    if (num_sectors > TRACK_MAX_SECTORS)
    {
        DBGd1("Error: Too many sectors requested", (int) num_sectors);
        num_sectors = TRACK_MAX_SECTORS;
    }

    self->tracknum = tracknum;
    self->num_sectors = num_sectors;
    self->free_sectors = num_sectors;

    pos.track = tracknum;
    for (i = 0; i < num_sectors; ++i)
    {
        pos.sector = i;
        self->sectors[i] = OBJNEW3(Block, self, &pos, &blockStatusChanged);
    }
    return self;
}

SOEXPORT void
Track_done(Track *self)
{
    unsigned int i;
    for (i = 0; i < self->num_sectors; ++i)
    {
        OBJDEL(Block, self->sectors[i]);
    }
}

SOEXPORT BlockStatus
Track_blockStatus(const Track *self, int sector)
{
    if (sector < 0 || sector >= (int) self->num_sectors)
        return (BlockStatus) -1;
    return Block_status(self->sectors[sector]);
}

SOEXPORT size_t
Track_numSectors(const Track *self)
{
    return self->num_sectors;
}

static int
_freeSectorsRaw(const Track *self, BlockStatus mask)
{
    int free = 0;
    unsigned int i;

    for (i = 0; i < self->num_sectors; ++i)
    {
        if (!(Block_status(self->sectors[i]) & ~mask)) ++free;
    }

    return free;
}

SOEXPORT int
Track_freeSectors(const Track *self, BlockStatus mask)
{
    if (mask) return _freeSectorsRaw(self, mask);
    return self->free_sectors;
}

SOEXPORT int
Track_reserveBlock(Track *self, int sector, IModule *by)
{
    if (sector < 0 || sector >= (int) self->num_sectors) return 0;
    return Block_reserve(self->sectors[sector], by);
}

SOEXPORT int
Track_allocateBlock(Track *self, int sector)
{
    if (sector < 0 || sector >= (int) self->num_sectors) return 0;
    return Block_allocate(self->sectors[sector]);
}

SOEXPORT int
Track_allocateFirstFreeFrom(Track *self, int sector, int askModules)
{
    Block *b;
    BlockStatus s;
    int i;

    if (sector < 0) return -1;

    for (i = self->num_sectors; i > 0 ; --i, ++sector)
    {
        if (sector >= (int) self->num_sectors) sector = 0;
        b = self->sectors[sector];
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
Track_block(const Track *self, int sector)
{
    if (sector < 0 || sector >= (int) self->num_sectors) return 0;
    return self->sectors[sector];
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
