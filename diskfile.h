#ifndef DISKFILE_H
#define DISKFILE_H

#include <mkd64/diskfile.h>
#include <stdio.h>

Diskfile *diskfile_new(void);
void diskfile_delete(Diskfile *this);

void diskfile_setFileNo(Diskfile *this, int fileNo);

int diskfile_readFromHost(Diskfile *this, FILE *hostfile);

int diskfile_write(Diskfile *this, Image *image,
        const BlockPosition *startPosition);

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
