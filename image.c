
#include <stdlib.h>

#include "image.h"
#include "track.h"
#include "block.h"

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
        pos->sector = 1;
        t = image_track(image, pos->track);
    }
    else
    {
        t = image_track(image, pos->track);
        pos->sector = ((pos->sector - 1 + interleave)
            % track_numSectors(t)) + 1;
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
        if (pos->sector > sectors) pos->sector = 1;
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
    Track *tracks[IMAGE_NUM_TRACKS];
};

Image *
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
    return this;
}

void
image_delete(Image *this)
{
    int i;

    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        track_delete(this->tracks[i]);
    }
    free(this);
}

BlockStatus
image_blockStatus(const Image *this, int track, int sector)
{
    Track *t = image_track(this, track);
    if (!t) return (BlockStatus) -1;
    return track_blockStatus(t, sector);
}

Track *
image_track(const Image *this, int track)
{
    if (track < 1 || track > IMAGE_NUM_TRACKS) return 0;
    return this->tracks[track-1];
}

Block *
image_block(const Image *this, BlockPosition *pos)
{
    Track *t = image_track(this, pos->track);
    if (!t) return 0;
    return track_block(t, pos->sector);
}

void
image_setAllocator(Image *this, IAllocateStrategy *allocator)
{
    this->allocator = allocator;
}

int
image_nextFileBlock(const Image *this, int interleave, BlockPosition *pos)
{
    return this->allocator->nextFileBlock(
            this->allocator, this, interleave, pos);
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
