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
FileMap_init(FileMap *this)
{
    memset(this, 0, sizeof(FileMap));
    return this;
}

SOLOCAL void
FileMap_done(FileMap *this)
{
    FileMapEntry *current, *next;

    for (current = this->first; current; current = next)
    {
        next = current->next;
        OBJDEL(DiskFile, current->file);
        free(current->startPosition);
        free(current);
    }
}

SOLOCAL void
FileMap_add(FileMap *this, DiskFile *file, const BlockPosition *pos)
{
    FileMapEntry *current;

    if (!this->first)
    {
        current = malloc(sizeof(FileMapEntry));
        this->first = current;
    }
    else
    {
        current = this->first;
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
FileMap_dump(const FileMap *this, FILE *out)
{
    static const char *unnamed = "[UNNAMED]";
    const char *name;
    FileMapEntry *current;

    for (current = this->first; current; current = current->next)
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
