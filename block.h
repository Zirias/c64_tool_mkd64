#ifndef BLOCK_H
#define BLOCK_H

#include <mkd64/block.h>

typedef void (*BlockStatusChangedHandler)(void *owner, Block *block,
        BlockStatus oldStatus, BlockStatus newStatus);

Block *block_new(void *owner,
        const BlockPosition *pos, BlockStatusChangedHandler handler);

void block_delete(Block *this);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
