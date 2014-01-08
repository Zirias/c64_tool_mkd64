#ifndef IMAGE_H
#define IMAGE_H

#include <mkd64/image.h>
#include <stdio.h>
#include "filemap.h"

Image *image_new(void);
void image_delete(Image *this);

Filemap *image_filemap(const Image *this);

void image_reset(Image *this);

int image_dump(const Image *this, FILE *out);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
