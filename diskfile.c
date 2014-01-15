#include <mkd64/common.h>
#include <mkd64/debug.h>
#include <mkd64/util.h>

#include "diskfile.h"
#include "image.h"
#include "block.h"
#include "modrepo.h"
#include "mkd64.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
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
DiskFile_init(DiskFile *this)
{
    memset(this, 0, sizeof(DiskFile));
    return this;
}

SOLOCAL void
DiskFile_done(DiskFile *this)
{
    DiskFileData *d, *tmp;

    d = this->extraData;
    while (d)
    {
        tmp = d;
        d = d->next;
        tmp->deleter(tmp->owner, tmp->data);
        free(tmp);
    }

    free(this->name);
    free(this->content);
}

static DiskFileData *
_createData(const void *owner, void *data, DataDelete deleter)
{
    DiskFileData *d = malloc(sizeof(DiskFileData));
    d->next = 0;
    d->owner = owner;
    d->data = data;
    d->deleter = deleter;
    return d;
}

SOEXPORT void
DiskFile_attachData(DiskFile *this, const void *owner, void *data,
        DataDelete deleter)
{
    DiskFileData *parent;

    if (!this->extraData)
    {
        this->extraData = _createData(owner, data, deleter);
        return;
    }

    for (parent = this->extraData; parent->next; parent = parent->next)
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
DiskFile_data(const DiskFile *this, const void *owner)
{
    DiskFileData *d;

    for (d = this->extraData; d; d = d->next)
    {
        if (d->owner == owner) return d->data;
    }
    return 0;
}

SOLOCAL int
DiskFile_readFromHost(DiskFile *this, const char *hostfile)
{
    static struct stat st;
    void *ptr;
    FILE *f;

    if (stat(hostfile, &st) < 0) return 0;
    if (st.st_size < 1) return 0;
    if(!(f = fopen(hostfile, "rb"))) return 0;

    this->size = (size_t) st.st_size;
    free(this->content);
    this->content = 0;

    ptr = malloc(this->size);
    
    if (fread(ptr, 1, this->size, f) != this->size)
    {
        this->size = 0;
        free(ptr);
        fclose(f);
        return 0;
    }

    fclose(f);
    this->content = ptr;
    return 1;
}

SOEXPORT size_t
DiskFile_size(const DiskFile *this)
{
    return this->size;
}

SOEXPORT size_t
DiskFile_blocks(const DiskFile *this)
{
    return this->blocks;
}

SOEXPORT void
DiskFile_setInterleave(DiskFile *this, int interleave)
{
    this->interleave = interleave;
}

SOEXPORT int
DiskFile_interleave(const DiskFile *this)
{
    return this->interleave;
}

SOEXPORT void
DiskFile_setName(DiskFile *this, const char *name)
{
    if (this->name) free(this->name);
    this->name = copyString(name);
}

SOEXPORT const char *
DiskFile_name(const DiskFile *this)
{
    return this->name;
}

static void
_rollbackWrite(DiskFile *this, Image *image, const BlockPosition *pos)
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
DiskFile_write(DiskFile *this, Image *image,
        const BlockPosition *startPosition)
{
    const BlockPosition inval = { 0, 0 };
    const BlockPosition *start, *current;
    IBlockAllocator *alloc = Image_allocator(image);
    uint8_t *contentPos = this->content;
    size_t toWrite = this->size;
    int writereserved = 0;

    uint8_t *blockData;
    Block *block, *nextBlock;
    size_t blockWrite;

    if (!toWrite)
    {
        start = &inval;
        goto DiskFile_write_done;
    }

    alloc->setInterleave(alloc, this->interleave);
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
    this->blocks = 1;

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
                _rollbackWrite(this, image, start);
                return 0;
            }
            current = Block_position(nextBlock);
            Block_setNextTrack(block, current->track);
            Block_setNextSector(block, current->sector);
            contentPos += blockWrite;
            ++(this->blocks);
        }
        else
        {
            Block_setNextTrack(block, 0);
            Block_setNextSector(block, blockWrite + 1);
        }
    } while (toWrite);

DiskFile_write_done:
    FileMap_add(Image_fileMap(image), this, start);

    ModRepo_allFileWritten(Mkd64_modRepo(MKD64), this, start);

    return 1;
}

SOLOCAL void
DiskFile_setFileNo(DiskFile *this, int fileNo)
{
    this->fileNo = fileNo;
}

SOEXPORT int
DiskFile_fileNo(const DiskFile *this)
{
    return this->fileNo;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
