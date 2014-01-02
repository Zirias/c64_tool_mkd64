#include <mkd64/common.h>

#include <stdlib.h>
#include <stdio.h>

#include <mkd64/track.h>
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
_nextFileBlock(void *this, const Image *image,
        int interleave, BlockPosition *pos)
{
    Track *t;
    size_t sectors;

    if (pos->track == 0)
    {
        pos->track = 17;
        pos->sector = 0;
        t = image_track(image, pos->track);
    }
    else
    {
        t = image_track(image, pos->track);
        if (track_blockStatus(t, pos->sector) != BS_NONE)
        {
            pos->sector = (pos->sector + interleave) % track_numSectors(t);
        }
    }

    while (t && !track_freeSectors(t))
    {
        if (pos->track == 1)
        {
            pos->track = 18;
        }
        else if (pos->track < 18)
        {
            --(pos->track);
        }
        else
        {
            ++(pos->track);
        }
        t = image_track(image, pos->track);
    }

    if (!t) return 0;

    sectors = track_numSectors(t);

    while (track_blockStatus(t, pos->sector) != BS_NONE)
    {
        ++(pos->sector);
        if (pos->sector >= sectors) pos->sector = 0;
    }

    track_allocateBlock(t, pos->sector);

    return 1;
}

static IAllocateStrategy _defaultAllocator =
{
    &_nextFileBlock
};

struct image
{
    size_t num_tracks;
    IAllocateStrategy *allocator;
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
    if (track < 1 || track > IMAGE_NUM_TRACKS) return 0;
    return this->tracks[track-1];
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
image_setAllocator(Image *this, IAllocateStrategy *allocator)
{
    this->allocator = allocator;
}

SOEXPORT int
image_nextFileBlock(const Image *this, int interleave, BlockPosition *pos)
{
    return this->allocator->nextFileBlock(
            this->allocator, this, interleave, pos);
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
