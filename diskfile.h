#ifndef MKD64_DISKFILE_H
#define MKD64_DISKFILE_H

#include "image.h"
#include "block.h"

#include <stdlib.h>
#include <stdio.h>

struct diskfile;
typedef struct diskfile Diskfile;

Diskfile *diskfile_new(void);
void diskfile_delete(Diskfile *this);

int diskfile_readFromHost(Diskfile *this, FILE *hostfile);

size_t diskfile_size(const Diskfile *this);
size_t diskfile_blocks(const Diskfile *this);

void diskfile_setInterleave(Diskfile *this, int interleave);
int diskfile_interleave(const Diskfile *this);

void diskfile_setName(Diskfile *this, const char *name);
const char *diskfile_name(const Diskfile *this);

int diskfile_write(Diskfile *this, Image *image,
        const BlockPosition *startPosition);

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
