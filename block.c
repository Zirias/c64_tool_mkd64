#include <mkd64/common.h>

#include <stdlib.h>
#include <string.h>

#include "block.h"

struct Block
{
    void *owner;
    IModule *reservedBy;
    BlockStatus status;
    BlockPosition pos;
    BlockStatusChangedHandler handler;
    uint8_t data[BLOCK_RAWSIZE];
};

SOLOCAL size_t
Block_objectSize(void)
{
    return sizeof(Block);
}

SOLOCAL Block *
Block_init(Block *this, void *owner,
        const BlockPosition *pos, BlockStatusChangedHandler handler)
{
    memset(this, 0, sizeof(Block));
    this->owner = owner;
    this->status = BS_NONE;
    this->pos.track = pos->track;
    this->pos.sector = pos->sector;
    this->handler = handler;
    return this;
}

SOLOCAL void
Block_done(Block *this)
{
}

SOEXPORT BlockStatus
Block_status(const Block *this)
{
    return this->status;
}

SOEXPORT const BlockPosition *
Block_position(const Block *this)
{
    return &(this->pos);
}

SOEXPORT uint8_t
Block_nextTrack(const Block *this)
{
    return this->data[0];
}

SOEXPORT uint8_t
Block_nextSector(const Block *this)
{
    return this->data[1];
}

SOEXPORT void
Block_nextPosition(const Block *this, BlockPosition *pos)
{
    pos->track = this->data[0];
    pos->sector = this->data[1];
}

SOEXPORT IModule *
Block_reservedBy(const Block *this)
{
    if (this->status & BS_RESERVED) return this->reservedBy;
    return 0;
}

SOEXPORT void
Block_setNextTrack(Block *this, uint8_t nextTrack)
{
    this->data[0] = nextTrack;
}

SOEXPORT void
Block_setNextSector(Block *this, uint8_t nextSector)
{
    this->data[1] = nextSector;
}

SOEXPORT void
Block_setNextPosition(Block *this, const BlockPosition *pos)
{
    this->data[0] = pos->track;
    this->data[1] = pos->sector;
}

SOEXPORT int
Block_reserve(Block *this, IModule *by)
{
    BlockStatus old;

    if (this->status & BS_RESERVED) return 0;
    old = this->status;
    this->status |= BS_RESERVED;
    this->reservedBy = by;
    if (this->handler)
    {
        this->handler(this->owner, this, old, this->status);
    }
    return 1;
}

SOEXPORT int
Block_unReserve(Block *this)
{
    BlockStatus old;

    if (!(this->status & BS_RESERVED)) return 0;

    if (this->reservedBy->requestReservedBlock &&
            this->reservedBy->requestReservedBlock(
                this->reservedBy, &(this->pos)))
    {
        old = this->status;
        this->status &= ~BS_RESERVED;
        if (this->handler)
        {
            this->handler(this->owner, this, old, this->status);
        }
        return 1;
    }
    return 0;
}

SOEXPORT int
Block_allocate(Block *this)
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

SOEXPORT int
Block_free(Block *this)
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

SOEXPORT uint8_t *
Block_data(Block *this)
{
    return &(this->data[2]);
}

SOEXPORT uint8_t *
Block_rawData(Block *this)
{
    return this->data;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
