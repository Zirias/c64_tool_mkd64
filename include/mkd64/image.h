#ifndef MKD64_IMAGE_H
#define MKD64_IMAGE_H

struct image;
typedef struct image Image;

#include <mkd64/track.h>
#include <mkd64/block.h>

typedef struct
{
    int (*nextFileBlock)(void *this, const Image *image,
            int interleave, BlockPosition *pos);
} IAllocateStrategy;

BlockStatus image_blockStatus(const Image *this, const BlockPosition *pos);

Track *image_track(const Image *this, int tracknum);
Block *image_block(const Image *this, BlockPosition *pos);

void image_setAllocator(Image *this, IAllocateStrategy *allocator);
int image_nextFileBlock(const Image *this, int interleave, BlockPosition *pos);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
