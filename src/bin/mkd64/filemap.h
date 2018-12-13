#ifndef FileMap_H
#define FileMap_H

#include <stdio.h>
#include <stdlib.h>

typedef struct FileMap FileMap;

#include <mkd64/diskfile.h>
#include <mkd64/block.h>

size_t FileMap_objectSize(void);
FileMap *FileMap_init(FileMap *self);
void FileMap_done(FileMap *self);

void FileMap_add(FileMap *self, DiskFile *file, const BlockPosition *pos);
int FileMap_dump(const FileMap *self, FILE *out);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
