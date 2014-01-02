#ifndef FILEMAP_H
#define FILEMAP_H

#include <stdio.h>

struct filemap;
typedef struct filemap Filemap;

#include <mkd64/diskfile.h>
#include <mkd64/block.h>

Filemap *filemap_new(void);
void filemap_delete(Filemap *this);

void filemap_add(Filemap *this, Diskfile *file, const BlockPosition *pos);
int filemap_dump(const Filemap *this, FILE *out);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
