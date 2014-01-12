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

struct diskfileData;
typedef struct diskfileData DiskfileData;

struct diskfileData
{
    DiskfileData *next;
    const void *owner;
    void *data;
    DataDelete deleter;
};

struct diskfile
{
    char *name;
    int fileNo;
    size_t size;
    size_t blocks;
    int interleave;

    uint8_t *content;

    DiskfileData *extraData;
};

SOLOCAL Diskfile *
diskfile_new(void)
{
    Diskfile *this = calloc(1, sizeof(Diskfile));
    return this;
}

SOLOCAL void
diskfile_delete(Diskfile *this)
{
    DiskfileData *d, *tmp;

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
    free(this);
}

static DiskfileData *
_createData(const void *owner, void *data, DataDelete deleter)
{
    DiskfileData *d = malloc(sizeof(DiskfileData));
    d->next = 0;
    d->owner = owner;
    d->data = data;
    d->deleter = deleter;
    return d;
}

SOEXPORT void
diskfile_attachData(Diskfile *this, const void *owner, void *data,
        DataDelete deleter)
{
    DiskfileData *parent;

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
diskfile_data(const Diskfile *this, const void *owner)
{
    DiskfileData *d;

    for (d = this->extraData; d; d = d->next)
    {
        if (d->owner == owner) return d->data;
    }
    return 0;
}

SOLOCAL int
diskfile_readFromHost(Diskfile *this, const char *hostfile)
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
diskfile_size(const Diskfile *this)
{
    return this->size;
}

SOEXPORT size_t
diskfile_blocks(const Diskfile *this)
{
    return this->blocks;
}

SOEXPORT void
diskfile_setInterleave(Diskfile *this, int interleave)
{
    this->interleave = interleave;
}

SOEXPORT int
diskfile_interleave(const Diskfile *this)
{
    return this->interleave;
}

SOEXPORT void
diskfile_setName(Diskfile *this, const char *name)
{
    if (this->name) free(this->name);
    this->name = copyString(name);
}

SOEXPORT const char *
diskfile_name(const Diskfile *this)
{
    return this->name;
}

static void
_rollbackWrite(Diskfile *this, Image *image, const BlockPosition *pos)
{
    Block *block;
    BlockPosition current;

    current.track = pos->track;
    current.sector = pos->sector;

    while (current.track)
    {
        block = image_block(image, &current);
        block_free(block);
        current.track = block_nextTrack(block);
        current.sector = block_nextSector(block);
    }
}

SOLOCAL int
diskfile_write(Diskfile *this, Image *image,
        const BlockPosition *startPosition)
{
    const BlockPosition inval = { 0, 0 };
    const BlockPosition *start, *current;
    IBlockAllocator *alloc = image_allocator(image);
    uint8_t *contentPos = this->content;
    size_t toWrite = this->size;
    int writereserved = 0;

    uint8_t *blockData;
    Block *block, *nextBlock;
    size_t blockWrite;

    if (!toWrite)
    {
        start = &inval;
        goto diskfile_write_done;
    }

    alloc->setInterleave(alloc, this->interleave);
    alloc->setConsiderReserved(alloc, 0);

    if (startPosition && startPosition->track)
    {
        /* fixed start position requested */
        nextBlock = image_allocateAt(image, startPosition);
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

    start = block_position(nextBlock);
    current = start;
    this->blocks = 1;

    do
    {
        block = nextBlock;
        blockWrite = (toWrite > BLOCK_SIZE) ? BLOCK_SIZE : toWrite;
        toWrite -= blockWrite;
        blockData = block_data(block);
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
                block_setNextTrack(block, 0);
                _rollbackWrite(this, image, start);
                return 0;
            }
            current = block_position(nextBlock);
            block_setNextTrack(block, current->track);
            block_setNextSector(block, current->sector);
            contentPos += blockWrite;
            ++(this->blocks);
        }
        else
        {
            block_setNextTrack(block, 0);
            block_setNextSector(block, blockWrite + 1);
        }
    } while (toWrite);

diskfile_write_done:
    filemap_add(image_filemap(image), this, start);

    modrepo_allFileWritten(mkd64_modrepo(), this, start);

    return 1;
}

SOLOCAL void
diskfile_setFileNo(Diskfile *this, int fileNo)
{
    this->fileNo = fileNo;
}

SOEXPORT int
diskfile_fileNo(const Diskfile *this)
{
    return this->fileNo;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
