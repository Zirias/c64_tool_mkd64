
#include <stdlib.h>

#include "image.h"
#include "track.h"
#include "block.h"
#include "blckstat.h"

#define IMAGE_NUM_TRACKS 35

static int num_sectors[] =
{
    21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
    19,19,19,19,19,19,19,
    18,18,18,18,18,18,
    17,17,17,17,17
};

struct image
{
    size_t num_tracks;
    Track *tracks[IMAGE_NUM_TRACKS];
};

static Track *
getTrack(const Image *this, int track)
{
    if (track < 1 || track > IMAGE_NUM_TRACKS) return 0;
    return this->tracks[track-1];
}

Image *
image_new(void)
{
    int i;

    Image *this = malloc(sizeof(Image));
    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        this->tracks[i] = track_new(num_sectors[i]);
    }
    this->num_tracks = IMAGE_NUM_TRACKS;
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
    Track *t = getTrack(this, track);
    if (!t) return (BlockStatus) -1;
    return track_blockStatus(t, sector);
}

Block *
image_block(Image *this, BlockPosition *pos)
{
    Track *t = getTrack(this, pos->track);
    if (!t) return 0;
    return track_block(t, pos->sector);
}

/* vim: et:si:ts=8:sts=4:sw=4
*/
