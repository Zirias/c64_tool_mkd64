#ifndef MKD64_DISKFILE_H
#define MKD64_DISKFILE_H

struct diskfile;
typedef struct diskfile Diskfile;

#include <mkd64/image.h>
#include <mkd64/block.h>
#include <stdlib.h>

size_t diskfile_size(const Diskfile *this);
size_t diskfile_blocks(const Diskfile *this);

void diskfile_setInterleave(Diskfile *this, int interleave);
int diskfile_interleave(const Diskfile *this);

void diskfile_setName(Diskfile *this, const char *name);
const char *diskfile_name(const Diskfile *this);

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
