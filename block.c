
#include <stdlib.h>
#include <string.h>

#include "block.h"

struct block
{
    int track;
    int sector;
    uint8_t data[256];
};

Block *
block_new(void)
{
    Block *this = calloc(1, sizeof(Block));
    return this;
}

Block *
block_newAt(int track, int sector)
{
    Block *this = block_new();
    block_setPosition(this, track, sector);
    return this;
}

Block *
block_newFrom(uint8_t data[256])
{
    Block *this = block_new();
    block_setData(this, data);
    return this;
}

Block *
block_newAtFrom(int track, int sector, uint8_t data[256])
{
    Block *this = block_newAt(track, sector);
    block_setData(this, data);
    return this;
}

int
Block_track(const Block *this)
{
    return this->track;
}

int
Block_sector(const Block *this)
{
    return this->sector;
}

uint8_t *
Block_data(const Block *this)
{
    uint8_t *data = malloc(256);
    memcpy(data, &(this->data), 256);
    return data;
}

int
Block_setPosition(Block *this, int track, int sector)
{
    this->track = track;
    this->sector = sector;
    return 1;
}

int
Block_setData(Block *this, uint8_t data[256])
{
    memcpy(&(this->data), data, 256);
    return 1;
}

/* vim: et:si:ts=8:sts=4:sw=4
*/
