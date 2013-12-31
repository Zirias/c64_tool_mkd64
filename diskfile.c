
#include "diskfile.h"
#include "image.h"
#include "block.h"
#include "debug.h"
#include "modrepo.h"
#include "mkd64.h"

#include <stdlib.h>
#include "stdintrp.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>

struct diskfile
{
    const char *name;
    size_t size;
    size_t blocks;
    int interleave;

    uint8_t *content;

    void *extra;
};

Diskfile *
diskfile_new(void)
{
    Diskfile *this = calloc(1, sizeof(Diskfile));
    return this;
}

void
diskfile_delete(Diskfile *this)
{
    free(this->content);
    free(this);
}

int
diskfile_readFromHost(Diskfile *this, FILE *hostfile)
{
    static struct stat st;
    void *ptr;

    if (fstat(fileno(hostfile), &st) < 0) return 0;
    if (st.st_size < 1) return 0;

    this->size = (size_t) st.st_size;
    free(this->content);
    this->content = 0;

    ptr = malloc(this->size);
    
    rewind(hostfile);
    if (fread(ptr, 1, this->size, hostfile) != this->size)
    {
        this->size = 0;
        free(ptr);
        return 0;
    }

    this->content = ptr;
    return 1;
}

size_t
diskfile_size(const Diskfile *this)
{
    return this->size;
}

size_t
diskfile_blocks(const Diskfile *this)
{
    return this->blocks;
}

void
diskfile_setInterleave(Diskfile *this, int interleave)
{
    this->interleave = interleave;
}

int
diskfile_interleave(const Diskfile *this)
{
    return this->interleave;
}

void
diskfile_setName(Diskfile *this, const char *name)
{
    this->name = name;
}

const char *
diskfile_name(const Diskfile *this)
{
    return this->name;
}

static void
_rollbackWrite(Diskfile *this, Image *image, BlockPosition *pos)
{
    Block *block;

    while (pos->track)
    {
        block = image_block(image, pos);
        block_free(block);
        pos->track = block_nextTrack(block);
        pos->sector = block_nextSector(block);
    }
}

int
diskfile_write(Diskfile *this, Image *image,
        const BlockPosition *startPosition)
{
    BlockPosition start = {0,0};
    BlockPosition current = {0,0};
    uint8_t *contentPos = this->content;
    size_t toWrite = this->size;

    uint8_t *blockData;
    Block *block;
    size_t blockWrite;

    if (startPosition && startPosition->track > 0)
    {
        if (image_blockStatus(image, startPosition) != BS_NONE)
        {
            return 0;
        }
        current.track = startPosition->track;
        current.sector = startPosition->sector;
    }

    image_nextFileBlock(image, this->interleave, &current);
    start.track = current.track;
    start.sector = current.sector;
    this->blocks = 1;

    do
    {
        block = image_block(image, &current);
        blockWrite = (toWrite > BLOCK_SIZE) ? BLOCK_SIZE : toWrite;
        toWrite -= blockWrite;
        blockData = block_data(block);
        DBGd2("writing file", current.track, current.sector);
        memcpy(blockData, contentPos, blockWrite);
        if (toWrite)
        {
            if (!image_nextFileBlock(image, this->interleave, &current))
            {
                block_setNextTrack(block, 0);
                _rollbackWrite(this, image, &start);
                return 0;
            }
            block_setNextTrack(block, current.track);
            block_setNextSector(block, current.sector);
            contentPos += blockWrite;
            ++(this->blocks);
        }
        else
        {
            block_setNextTrack(block, 0);
            block_setNextSector(block, blockWrite);
        }
    } while (toWrite);

    filemap_add(image_filemap(image), this, &start);

    modrepo_allFileWritten(mkd64_modrepo(), this, &start);

    return 1;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
