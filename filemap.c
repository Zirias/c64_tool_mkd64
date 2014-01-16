#include <mkd64/common.h>

#include "filemap.h"
#include "diskfile.h"
#include "block.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct FileMapEntry FileMapEntry;

struct FileMapEntry
{
    FileMapEntry *next;
    DiskFile *file;
    BlockPosition *startPosition;
};

struct FileMap
{
    FileMapEntry *first;
};

SOLOCAL size_t
FileMap_objectSize(void)
{
    return sizeof(FileMap);
}

SOLOCAL FileMap *
FileMap_init(FileMap *self)
{
    memset(self, 0, sizeof(FileMap));
    return self;
}

SOLOCAL void
FileMap_done(FileMap *self)
{
    FileMapEntry *current, *next;

    for (current = self->first; current; current = next)
    {
        next = current->next;
        OBJDEL(DiskFile, current->file);
        free(current->startPosition);
        free(current);
    }
}

SOLOCAL void
FileMap_add(FileMap *self, DiskFile *file, const BlockPosition *pos)
{
    FileMapEntry *current;

    if (!self->first)
    {
        current = malloc(sizeof(FileMapEntry));
        self->first = current;
    }
    else
    {
        current = self->first;
        while (current->next) current = current->next;
        current->next = malloc(sizeof(FileMapEntry));
        current = current->next;
    }
    current->next = 0;
    current->file = file;
    current->startPosition = malloc(sizeof(BlockPosition));
    current->startPosition->track = pos->track;
    current->startPosition->sector = pos->sector;
}

SOLOCAL int
FileMap_dump(const FileMap *self, FILE *out)
{
    static const char *unnamed = "[UNNAMED]";
    const char *name;
    FileMapEntry *current;

    for (current = self->first; current; current = current->next)
    {
        name = DiskFile_name(current->file);
        if (!name) name = unnamed;
        if (fprintf(out, "%u;%u;%s\n",
                current->startPosition->track,
                current->startPosition->sector,
                name) < 0)
            return 0;
    }

    return 1;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
