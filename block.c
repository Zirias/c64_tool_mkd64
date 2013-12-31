
#include "stdintrp.h"
#include <stdlib.h>

#include "block.h"

struct block
{
    void *owner;
    BlockStatus status;
    BlockPosition pos;
    BlockStatusChangedHandler handler;
    uint8_t data[BLOCK_RAWSIZE];
};

Block *
block_new(void *owner,
        const BlockPosition *pos, BlockStatusChangedHandler handler)
{
    Block *this = calloc(1, sizeof(Block));
    this->owner = owner;
    this->pos.track = pos->track;
    this->pos.sector = pos->sector;
    this->handler = handler;
    return this;
}

void
block_delete(Block *this)
{
    free(this);
}

BlockStatus
block_status(const Block *this)
{
    return this->status;
}

const BlockPosition *
block_position(const Block *this)
{
    return &(this->pos);
}

uint8_t
block_nextTrack(const Block *this)
{
    return this->data[0];
}

uint8_t
block_nextSector(const Block *this)
{
    return this->data[1];
}

void
block_nextPosition(const Block *this, BlockPosition *pos)
{
    pos->track = this->data[0];
    pos->sector = this->data[1];
}

void
block_setNextTrack(Block *this, uint8_t nextTrack)
{
    this->data[0] = nextTrack;
}

void
block_setNextSector(Block *this, uint8_t nextSector)
{
    this->data[1] = nextSector;
}

void
block_setNextPosition(Block *this, const BlockPosition *pos)
{
    this->data[0] = pos->track;
    this->data[1] = pos->sector;
}

int
block_reserve(Block *this)
{
    BlockStatus old;

    if (this->status & BS_RESERVED) return 0;
    old = this->status;
    this->status |= BS_RESERVED;
    if (this->handler)
    {
        this->handler(this->owner, this, old, this->status);
    }
    return 1;
}

int
block_unReserve(Block *this)
{
    BlockStatus old;

    if (!(this->status & BS_RESERVED)) return 0;
    old = this->status;
    this->status &= ~BS_RESERVED;
    if (this->handler)
    {
        this->handler(this->owner, this, old, this->status);
    }
    return 1;
}

int
block_allocate(Block *this)
{
    BlockStatus old;

    if (this->status & BS_ALLOCATED) return 0;
    old = this->status;
    this->status |= BS_ALLOCATED;
    if (this->handler)
    {
        this->handler(this->owner, this, old, this->status);
    }
    return 1;
}

int
block_free(Block *this)
{
    BlockStatus old;

    if (!(this->status & BS_ALLOCATED)) return 0;
    old = this->status;
    this->status &= ~BS_ALLOCATED;
    if (this->handler)
    {
        this->handler(this->owner, this, old, this->status);
    }
    return 1;
}

uint8_t *
block_data(Block *this)
{
    return &(this->data[2]);
}

uint8_t *
block_rawData(Block *this)
{
    return this->data;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
