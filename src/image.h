#ifndef IMAGE_H
#define IMAGE_H

#include <mkd64/image.h>
#include <stdio.h>
#include <stdlib.h>
#include "filemap.h"

size_t Image_objectSize(void);
Image *Image_init(Image *self);
void Image_done(Image *self);

FileMap *Image_fileMap(const Image *self);

int Image_dump(const Image *self, FILE *out);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
