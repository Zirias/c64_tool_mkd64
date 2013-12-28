
#include <stdlib.h>

#include "image.h"
#include "track.h"

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
    Track *tracks[IMAGE_NUM_TRACKS];
};

Image *
image_new(void)
{
    int i;

    Image *this = malloc(sizeof(Image));
    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        this->tracks[i] = track_new(num_sectors[i]);
    }
    return this;
}

void
image_delete(Image *this)
{
    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        track_delete(this->tracks[i]);
    }
    free(this);
}

int
image_isFree(const Image *this, int track, int sector)
{
    return 0;
}

int
image_readBlock(const Image *this, Block *block)
{
    return 0;
}

int
image_writeBlock(Image *this, const Block *block)
{
    return 0;
}

/* vim: et:si:ts=8:sts=4:sw=4
*/
