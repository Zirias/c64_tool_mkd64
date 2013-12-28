
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "track.h"
#include "block.h"

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
    int sector = block_sector(block);
    if (sector < 1 || sector > this->num_sectors) return 0;
    uint8_t *ptr = &data[(sector-1) * 256];
    block_setData(block, ptr);
    return 1;
}

int
track_writeBlock(Track *this, const Block *block)
{
    int sector = block_sector(block);
    if (sector < 1 || sector > this->num_sectors) return 0;
    uint8_t *ptr = &data[(sector-1) * 256];
    memcpy(ptr, block_data(block), 256);
    return 1;
}

/* vim: et:si:ts=8:sts=4:sw=4
*/
