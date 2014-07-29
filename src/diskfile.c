#include <mkd64/common.h>
#include <mkd64/debug.h>
#include <mkd64/util.h>

#include "diskfile.h"
#include "image.h"
#include "block.h"
#include "modrepo.h"
#include "mkd64.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct DiskFileData DiskFileData;

struct DiskFileData
{
    DiskFileData *next;
    const void *owner;
    void *data;
    DataDelete deleter;
};

struct DiskFile
{
    char *name;
    int fileNo;
    size_t size;
    size_t blocks;
    int interleave;

    uint8_t *content;

    DiskFileData *extraData;
};

SOLOCAL size_t
DiskFile_objectSize(void)
{
    return sizeof(DiskFile);
}

SOLOCAL DiskFile *
DiskFile_init(DiskFile *self)
{
    memset(self, 0, sizeof(DiskFile));
    return self;
}

SOLOCAL void
DiskFile_done(DiskFile *self)
{
    DiskFileData *d, *tmp;

    d = self->extraData;
    while (d)
    {
        tmp = d;
        d = d->next;
        tmp->deleter(tmp->owner, tmp->data);
        free(tmp);
    }

    free(self->name);
    free(self->content);
}

static DiskFileData *
_createData(const void *owner, void *data, DataDelete deleter)
{
    DiskFileData *d = mkd64Alloc(sizeof(DiskFileData));
    d->next = 0;
    d->owner = owner;
    d->data = data;
    d->deleter = deleter;
    return d;
}

SOEXPORT void
DiskFile_attachData(DiskFile *self, const void *owner, void *data,
        DataDelete deleter)
{
    DiskFileData *parent;

    if (!self->extraData)
    {
        self->extraData = _createData(owner, data, deleter);
        return;
    }

    for (parent = self->extraData; parent->next; parent = parent->next)
    {
        if (parent->owner == owner)
        {
            fputs("Warning: Same owner tries to attach data to the same file "
                    "twice,\n         deleting previous instance.\n", stderr);
            parent->deleter(parent->owner, parent->data);
            parent->data = data;
            parent->deleter = deleter;
            return;
        }
    }

    parent->next = _createData(owner, data, deleter);
}

SOEXPORT void *
DiskFile_data(const DiskFile *self, const void *owner)
{
    DiskFileData *d;

    for (d = self->extraData; d; d = d->next)
    {
        if (d->owner == owner) return d->data;
    }
    return 0;
}

SOLOCAL int
DiskFile_readFromHost(DiskFile *self, const char *hostfile)
{
    void *ptr;
    FILE *f;
    int64_t size;

    if(!(f = fopen(hostfile, "rb"))) return 0;

    size = getFileSize(f);
    if (size < 1 || size > (int64_t)SIZE_MAX)
    {
        fclose(f);
        return 0;
    }
    self->size = (size_t)size;

    free(self->content);
    self->content = 0;

    ptr = mkd64Alloc(self->size);
    
    if (fread(ptr, 1, self->size, f) != self->size)
    {
        self->size = 0;
        free(ptr);
        fclose(f);
        return 0;
    }

    fclose(f);
    self->content = ptr;
    return 1;
}

SOEXPORT size_t
DiskFile_size(const DiskFile *self)
{
    return self->size;
}

SOEXPORT size_t
DiskFile_blocks(const DiskFile *self)
{
    return self->blocks;
}

SOEXPORT void
DiskFile_setInterleave(DiskFile *self, int interleave)
{
    self->interleave = interleave;
}

SOEXPORT int
DiskFile_interleave(const DiskFile *self)
{
    return self->interleave;
}

SOEXPORT void
DiskFile_setName(DiskFile *self, const char *name)
{
    if (self->name) free(self->name);
    self->name = copyString(name);
}

SOEXPORT const char *
DiskFile_name(const DiskFile *self)
{
    return self->name;
}

static void
_rollbackWrite(Image *image, const BlockPosition *pos)
{
    Block *block;
    BlockPosition current;

    current.track = pos->track;
    current.sector = pos->sector;

    while (current.track)
    {
        block = Image_block(image, &current);
        Block_free(block);
        current.track = Block_nextTrack(block);
        current.sector = Block_nextSector(block);
    }
}

SOLOCAL int
DiskFile_write(DiskFile *self, Image *image,
        const BlockPosition *startPosition)
{
    const BlockPosition inval = { 0, 0 };
    const BlockPosition *start, *current;
    IBlockAllocator *alloc = Image_allocator(image);
    uint8_t *contentPos = self->content;
    size_t toWrite = self->size;
    int writereserved = 0;

    uint8_t *blockData;
    Block *block, *nextBlock;
    size_t blockWrite;

    if (!toWrite)
    {
        start = &inval;
        goto DiskFile_write_done;
    }

    alloc->setInterleave(alloc, self->interleave);
    alloc->setConsiderReserved(alloc, 0);

    if (startPosition && startPosition->track)
    {
        /* fixed start position requested */
        nextBlock = Image_allocateAt(image, startPosition);
    }
    else
    {
        /* automatic start position requested, use allocator */
        nextBlock = alloc->allocFirstBlock(alloc);
        if (!nextBlock)
        {
            writereserved = 1;
            alloc->setConsiderReserved(alloc, 1);
            nextBlock = alloc->allocFirstBlock(alloc);
        }
    }

    if (!nextBlock) return 0;

    start = Block_position(nextBlock);
    current = start;
    self->blocks = 1;

    do
    {
        block = nextBlock;
        blockWrite = (toWrite > BLOCK_SIZE) ? BLOCK_SIZE : toWrite;
        toWrite -= blockWrite;
        blockData = Block_data(block);
        DBGd2("writing file", current->track, current->sector);
        memcpy(blockData, contentPos, blockWrite);
        if (toWrite)
        {
            nextBlock = alloc->allocNextBlock(alloc, current);
            if (!nextBlock && !writereserved)
            {
                writereserved = 1;
                alloc->setConsiderReserved(alloc, 1);
                nextBlock = alloc->allocNextBlock(alloc, current);
            }
            if (!nextBlock)
            {
                Block_setNextTrack(block, 0);
                _rollbackWrite(image, start);
                return 0;
            }
            current = Block_position(nextBlock);
            Block_setNextTrack(block, current->track);
            Block_setNextSector(block, current->sector);
            contentPos += blockWrite;
            ++(self->blocks);
        }
        else
        {
            Block_setNextTrack(block, 0);
            Block_setNextSector(block, blockWrite + 1);
        }
    } while (toWrite);

DiskFile_write_done:
    FileMap_add(Image_fileMap(image), self, start);

    ModRepo_allFileWritten(Mkd64_modRepo(MKD64), self, start);

    return 1;
}

SOLOCAL void
DiskFile_setFileNo(DiskFile *self, int fileNo)
{
    self->fileNo = fileNo;
}

SOEXPORT int
DiskFile_fileNo(const DiskFile *self)
{
    return self->fileNo;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
