#ifndef DISKFILE_H
#define DISKFILE_H

#include <mkd64/diskfile.h>
#include <stdio.h>

size_t DiskFile_objectSize(void);
DiskFile *DiskFile_init(DiskFile *self);
void DiskFile_done(DiskFile *self);

void DiskFile_setFileNo(DiskFile *self, int fileNo);

int DiskFile_readFromHost(DiskFile *self, const char *hostfile);

/** write the file to the disk image
 * ATTENTION: This method transfers ownership of the DiskFile to the Image
 * after a successful write only. Therefore it's important to check the return
 * value.
 * @param self the diskFile
 * @param image the image to write the file to
 * @param startPosition an optional fixed track/sector position where to start
 * the file on the disk. If this is null or the track is 0, the file is placed
 * automatically.
 * @return 1 on success, 0 on error (disk full)
 */
int DiskFile_write(DiskFile *self, Image *image,
        const BlockPosition *startPosition);

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
