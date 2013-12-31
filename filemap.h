#ifndef MKD64_FILEMAP_H
#define MKD64_FILEMAP_H

#include "diskfile.h"
#include "block.h"

#include <stdio.h>

struct filemap;
typedef struct filemap Filemap;

Filemap *filemap_new(void);
void filemap_delete(Filemap *this);

void filemap_add(Filemap *this, const Diskfile *file, const BlockPosition *pos);
void filemap_dump(const Filemap *this, FILE *out);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
