#include <mkd64/common.h>

#include <stdlib.h>
#include <stdio.h>

#include <mkd64/mkd64.h>
#include <mkd64/track.h>
#include "modrepo.h"
#include "image.h"
#include "block.h"
#include "filemap.h"
#include "defalloc.h"

#define IMAGE_NUM_TRACKS 35

static size_t num_sectors[] =
{
    21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
    19,19,19,19,19,19,19,
    18,18,18,18,18,18,
    17,17,17,17,17
};

struct image
{
    size_t num_tracks;
    IBlockAllocator *allocator;
    Filemap *map;
    Track *tracks[IMAGE_NUM_TRACKS];
};

static void _initImage(Image *this)
{
    int i;

    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        this->tracks[i] = track_new(i+1, num_sectors[i]);
    }
    this->num_tracks = IMAGE_NUM_TRACKS;
    this->allocator = &defaultAllocator;
    this->map = filemap_new();
    defaultAllocator.setImage(&defaultAllocator, this);
}

SOLOCAL Image *
image_new(void)
{
    Image *this = malloc(sizeof(Image));
    _initImage(this);
    return this;
}

static void _cleanupImage(Image *this)
{
    int i;

    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        track_delete(this->tracks[i]);
    }
    filemap_delete(this->map);
}

SOLOCAL void
image_delete(Image *this)
{
    _cleanupImage(this);
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
image_block(const Image *this, const BlockPosition *pos)
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
    allocator->setImage(allocator, this);
    this->allocator = allocator;
}

SOEXPORT IBlockAllocator *
image_allocator(Image *this)
{
    return this->allocator;
}

SOLOCAL void
image_reset(Image *this)
{
    _cleanupImage(this);
    _initImage(this);
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
