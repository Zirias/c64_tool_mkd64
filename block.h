#ifndef BLOCK_H
#define BLOCK_H

#include <mkd64/block.h>
#include <stdlib.h>

typedef void (*BlockStatusChangedHandler)(void *owner, const Block *block,
        BlockStatus oldStatus, BlockStatus newStatus);

size_t Block_objectSize(void);

Block *Block_init(Block *this, void *owner,
        const BlockPosition *pos, BlockStatusChangedHandler handler);

void Block_done(Block *this);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
