#ifndef MKD64_IMAGE_H
#define MKD64_IMAGE_H

#include "block.h"

struct image;
typedef struct image Image;

Image *image_new(void);
void image_delete(Image *this);

BlockStatus image_blockStatus(const Image *this, int track, int sector);

Block *image_block(Image *this, BlockPosition *pos);

int image_nextFreeBlock(Image *this, BlockPosition *pos);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
