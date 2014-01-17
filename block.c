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
Block_init(Block *self, void *owner,
        const BlockPosition *pos, BlockStatusChangedHandler handler)
{
    memset(self, 0, sizeof(Block));
    self->owner = owner;
    self->status = BS_NONE;
    self->pos.track = pos->track;
    self->pos.sector = pos->sector;
    self->handler = handler;
    return self;
}

SOLOCAL void
Block_done(Block *self)
{
    (void) self; /* unused */
}

SOEXPORT BlockStatus
Block_status(const Block *self)
{
    return self->status;
}

SOEXPORT const BlockPosition *
Block_position(const Block *self)
{
    return &(self->pos);
}

SOEXPORT uint8_t
Block_nextTrack(const Block *self)
{
    return self->data[0];
}

SOEXPORT uint8_t
Block_nextSector(const Block *self)
{
    return self->data[1];
}

SOEXPORT void
Block_nextPosition(const Block *self, BlockPosition *pos)
{
    pos->track = self->data[0];
    pos->sector = self->data[1];
}

SOEXPORT IModule *
Block_reservedBy(const Block *self)
{
    if (self->status & BS_RESERVED) return self->reservedBy;
    return 0;
}

SOEXPORT void
Block_setNextTrack(Block *self, uint8_t nextTrack)
{
    self->data[0] = nextTrack;
}

SOEXPORT void
Block_setNextSector(Block *self, uint8_t nextSector)
{
    self->data[1] = nextSector;
}

SOEXPORT void
Block_setNextPosition(Block *self, const BlockPosition *pos)
{
    self->data[0] = pos->track;
    self->data[1] = pos->sector;
}

SOEXPORT int
Block_reserve(Block *self, IModule *by)
{
    BlockStatus old;

    if (self->status & BS_RESERVED) return 0;
    old = self->status;
    self->status |= BS_RESERVED;
    self->reservedBy = by;
    if (self->handler)
    {
        self->handler(self->owner, self, old, self->status);
    }
    return 1;
}

SOEXPORT int
Block_unReserve(Block *self)
{
    BlockStatus old;

    if (!(self->status & BS_RESERVED)) return 0;

    if (self->reservedBy->requestReservedBlock &&
            self->reservedBy->requestReservedBlock(
                self->reservedBy, &(self->pos)))
    {
        old = self->status;
        self->status &= ~BS_RESERVED;
        if (self->handler)
        {
            self->handler(self->owner, self, old, self->status);
        }
        return 1;
    }
    return 0;
}

SOEXPORT int
Block_allocate(Block *self)
{
    BlockStatus old;

    if (self->status & BS_ALLOCATED) return 0;
    old = self->status;
    self->status |= BS_ALLOCATED;
    if (self->handler)
    {
        self->handler(self->owner, self, old, self->status);
    }
    return 1;
}

SOEXPORT int
Block_free(Block *self)
{
    BlockStatus old;

    if (!(self->status & BS_ALLOCATED)) return 0;
    old = self->status;
    self->status &= ~BS_ALLOCATED;
    if (self->handler)
    {
        self->handler(self->owner, self, old, self->status);
    }
    return 1;
}

SOEXPORT uint8_t *
Block_data(Block *self)
{
    return &(self->data[2]);
}

SOEXPORT uint8_t *
Block_rawData(Block *self)
{
    return self->data;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
