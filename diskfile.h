#ifndef DISKFILE_H
#define DISKFILE_H

#include <mkd64/diskfile.h>
#include <stdio.h>

size_t DiskFile_objectSize(void);
DiskFile *DiskFile_init(DiskFile *self);
void DiskFile_done(DiskFile *self);

void DiskFile_setFileNo(DiskFile *self, int fileNo);

int DiskFile_readFromHost(DiskFile *self, const char *hostfile);

int DiskFile_write(DiskFile *self, Image *image,
        const BlockPosition *startPosition);

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
