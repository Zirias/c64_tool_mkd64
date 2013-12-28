#ifndef MKD64_DISKFILE_H
#define MKD64_DISKFILE_H

#include <stdlib.h>
#include <stdio.h>

struct diskfile;
typedef struct diskfile Diskfile;

Diskfile *diskfile_new(void);
void diskfile_delete(Diskfile *this);

int diskfile_readFromHost(Diskfile *this, FILE *hostfile);

size_t diskfile_size(const Diskfile *this);
size_t diskfile_blocks(const Diskfile *this);
int diskfile_startTrack(const Diskfile *this);
int diskfile_startSector(const Diskfile *this);

void diskfile_setInterleave(Diskfile *this, int interleave);

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
