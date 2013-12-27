#ifndef MKD64_IMAGE_H
#define MKD64_IMAGE_H

#include "block.h"

struct image;
typedef struct image Image;

Image *image_new(void);
void image_delete(Image *this);

int image_isFree(const Image *this, int track, int sector);
int image_readBlock(const Image *this, Block *block);
int image_writeBlock(Image *this, const Block *block);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
