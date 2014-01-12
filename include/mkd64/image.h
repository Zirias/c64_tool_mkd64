#ifndef MKD64_IMAGE_H
#define MKD64_IMAGE_H

#include <mkd64/common.h>

struct image;
typedef struct image Image;

#include <mkd64/image.h>
#include <mkd64/block.h>
#include <mkd64/track.h>
#include <mkd64/iblalloc.h>

DECLEXPORT BlockStatus image_blockStatus(const Image *this,
        const BlockPosition *pos);

DECLEXPORT Track *image_track(const Image *this, int tracknum);
DECLEXPORT Block *image_block(const Image *this, const BlockPosition *pos);

DECLEXPORT Block *image_allocateAt(const Image *this, const BlockPosition *pos);

DECLEXPORT void image_setAllocator(Image *this, IBlockAllocator *allocator);
DECLEXPORT IBlockAllocator *image_allocator(const Image *this);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
