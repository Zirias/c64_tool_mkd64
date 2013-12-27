
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "track.h"

#define TRACK_DATA_SIZE(x) (256 * x)
#define TRACK_SIZE(x) (sizeof(Track) - 1 + TRACK_DATA_SIZE(x))

struct track
{
    size_t num_sectors;
    uint8_t data[1];
};

Track *
track_new(size_t num_sectors)
{
    Track *this = malloc(TRACK_SIZE(num_sectors));
    this->num_sectors = num_sectors;
    memset(&(this->data), 0, TRACK_DATA_SIZE(num_sectors));
    return this;
}

void
track_delete(Track *this)
{
    free(this);
}

int
track_readBlock(const Track *this, Block *block)
{
    return 0;
}

int
track_writeBlock(Track *this, const Block *block)
{
    return 0;
}

/* vim: et:si:ts=8:sts=4:sw=4
*/
