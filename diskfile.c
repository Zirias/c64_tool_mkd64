
#include "diskfile.h"

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdio.h>

struct diskfile
{
    size_t size;
    size_t blocks;
    int interleave;
    int startTrack;
    int startSector;

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

int
diskfile_startTrack(const Diskfile *this)
{
    return this->startTrack;
}

int
diskfile_startSector(const Diskfile *this)
{
    return this->startSector;
}

void
diskfile_setInterleave(Diskfile *this, int interleave)
{
    this->interleave = interleave;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
