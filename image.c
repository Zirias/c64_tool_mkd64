
#include <stdlib.h>

#include "image.h"
#include "track.h"

struct image
{
    Track *tracks[35];
};

Image *
image_new(void)
{
    Image *this = calloc(1, sizeof(Image));
    return this;
}

void
image_delete(Image *this)
{
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
