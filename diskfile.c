
#include "diskfile.h"
#include "image.h"
#include "block.h"

#include <stdlib.h>
#include "stdintrp.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>

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

int
diskfile_write(Diskfile *this, Image *image,
        const BlockPosition *startPosition)
{
    BlockPosition start = {0,0};
    BlockPosition current = {0,0};
    Block *block;
    uint8_t *contentPos;

    contentPos = this->content;

    if (startPosition && startPosition->track > 0)
    {
        start.track = current.track = startPosition->track;
        start.sector = current.sector = startPosition->sector;
    }

    if (!image_nextFileBlock(image, this->interleave, &current)) return 0;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
