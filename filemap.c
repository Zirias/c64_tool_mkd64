#include <mkd64/common.h>

#include "filemap.h"
#include "diskfile.h"
#include "block.h"

#include <stdlib.h>
#include <stdio.h>

struct filemapEntry;
typedef struct filemapEntry FilemapEntry;

struct filemapEntry
{
    FilemapEntry *next;
    Diskfile *file;
    BlockPosition *startPosition;
};

struct filemap
{
    FilemapEntry *first;
};

SOLOCAL Filemap *
filemap_new(void)
{
    Filemap *this = calloc(1, sizeof(Filemap));
    return this;
}

SOLOCAL void
filemap_delete(Filemap *this)
{
    FilemapEntry *current, *next;

    for (current = this->first; current; current = next)
    {
        next = current->next;
        diskfile_delete(current->file);
        free(current->startPosition);
        free(current);
    }
    free(this);
}

SOLOCAL void
filemap_add(Filemap *this, Diskfile *file, const BlockPosition *pos)
{
    FilemapEntry *current;

    if (!this->first)
    {
        current = malloc(sizeof(FilemapEntry));
        this->first = current;
    }
    else
    {
        current = this->first;
        while (current->next) current = current->next;
        current->next = malloc(sizeof(FilemapEntry));
        current = current->next;
    }
    current->next = 0;
    current->file = file;
    current->startPosition = malloc(sizeof(BlockPosition));
    current->startPosition->track = pos->track;
    current->startPosition->sector = pos->sector;
}

SOLOCAL int
filemap_dump(const Filemap *this, FILE *out)
{
    static const char *unnamed = "[UNNAMED]";
    const char *name;
    FilemapEntry *current;

    for (current = this->first; current; current = current->next)
    {
        name = diskfile_name(current->file);
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
