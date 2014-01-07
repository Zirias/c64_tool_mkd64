#include <mkd64/common.h>

#include <stdlib.h>
#include <stdio.h>

#include <mkd64/mkd64.h>
#include <mkd64/track.h>
#include "modrepo.h"
#include "image.h"
#include "block.h"
#include "filemap.h"

#define IMAGE_NUM_TRACKS 35

static size_t num_sectors[] =
{
    21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
    19,19,19,19,19,19,19,
    18,18,18,18,18,18,
    17,17,17,17,17
};

static int
_nextFileBlock(IBlockAllocator *this, Image *image, int interleave,
        BlockPosition *pos, int considerReserved)
{
    BlockPosition currentp = { pos->track, pos->sector };
    BlockStatus s;
    Track *t;
    int foundSector;

    if (currentp.track == 0)
    {
        currentp.track = 17;
        currentp.sector = 0;
        t = image_track(image, currentp.track);
    }
    else
    {
        t = image_track(image, currentp.track);
        s = track_blockStatus(t, currentp.sector);
        if (considerReserved) s |= ~BS_RESERVED;
        if (s != BS_NONE)
        {
            currentp.sector =
                (currentp.sector + interleave) % track_numSectors(t);
        }
    }

    while (t && (foundSector = track_allocateFirstFreeFrom(t,
                    currentp.sector, considerReserved)) < 0)
    {
        if (currentp.track == 1)
        {
            currentp.track = 18;
        }
        else if (currentp.track < 18)
        {
            --(currentp.track);
        }
        else
        {
            ++(currentp.track);
        }
        t = image_track(image, currentp.track);
    }

    if (!t) return 0;

    pos->track = currentp.track;
    pos->sector = foundSector;

    return 1;
}

static IBlockAllocator _defaultAllocator =
{
    &_nextFileBlock
};

struct image
{
    size_t num_tracks;
    IBlockAllocator *allocator;
    Filemap *map;
    Track *tracks[IMAGE_NUM_TRACKS];
};

SOLOCAL Image *
image_new(void)
{
    int i;

    Image *this = malloc(sizeof(Image));
    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        this->tracks[i] = track_new(i+1, num_sectors[i]);
    }
    this->num_tracks = IMAGE_NUM_TRACKS;
    this->allocator = &_defaultAllocator;
    this->map = filemap_new();
    return this;
}

SOLOCAL void
image_delete(Image *this)
{
    int i;

    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        track_delete(this->tracks[i]);
    }
    filemap_delete(this->map);
    free(this);
}

SOEXPORT BlockStatus
image_blockStatus(const Image *this, const BlockPosition *pos)
{
    Track *t = image_track(this, pos->track);
    if (!t) return (BlockStatus) -1;
    return track_blockStatus(t, pos->sector);
}

SOEXPORT Track *
image_track(const Image *this, int track)
{
    Track *t;

    if (track > 0 && track <= IMAGE_NUM_TRACKS)
    {
        t = this->tracks[track-1];
    }
    else
    {
        t = modrepo_firstGetTrack(mkd64_modrepo(), track);
    }

    return t;
}

SOEXPORT Block *
image_block(const Image *this, BlockPosition *pos)
{
    Track *t = image_track(this, pos->track);
    if (!t) return 0;
    return track_block(t, pos->sector);
}

SOLOCAL Filemap *
image_filemap(const Image *this)
{
    return this->map;
}

SOEXPORT void
image_setAllocator(Image *this, IBlockAllocator *allocator)
{
    this->allocator = allocator;
}

SOEXPORT int
image_nextFileBlock(Image *this, int interleave, BlockPosition *pos)
{
    if (this->allocator->nextFileBlock(
                this->allocator, this, interleave, pos, 0))
    {
        return 1;
    }
    else
    {
        return this->allocator->nextFileBlock(
                this->allocator, this, interleave, pos, 1);
    }
}

SOLOCAL int
image_dump(const Image *this, FILE *out)
{
    int tracknum = 0;
    Track *track;
    Block *block;
    size_t num_sectors;
    int i;

    while ((track = image_track(this, ++tracknum)))
    {
        num_sectors = track_numSectors(track);
        for (i = 0; i < num_sectors; ++i)
        {
            block = track_block(track, i);
            if (fwrite(block_rawData(block), BLOCK_RAWSIZE, 1, out) != 1)
                return 0;
        }
    }
    return 1;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
