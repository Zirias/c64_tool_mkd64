#ifndef FileMap_H
#define FileMap_H

#include <stdio.h>
#include <stdlib.h>

typedef struct FileMap FileMap;

#include <mkd64/diskfile.h>
#include <mkd64/block.h>

size_t FileMap_objectSize(void);
FileMap *FileMap_init(FileMap *this);
void FileMap_done(FileMap *this);

void FileMap_add(FileMap *this, DiskFile *file, const BlockPosition *pos);
int FileMap_dump(const FileMap *this, FILE *out);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
