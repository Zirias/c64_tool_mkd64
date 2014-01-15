#include <mkd64/common.h>

#include <stdlib.h>
#include <stdio.h>

#include <mkd64/mkd64.h>
#include <mkd64/track.h>
#include "modrepo.h"
#include "image.h"
#include "block.h"
#include "filemap.h"
#include "defalloc.h"

#define IMAGE_NUM_TRACKS 35

static size_t num_sectors[] =
{
    21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
    19,19,19,19,19,19,19,
    18,18,18,18,18,18,
    17,17,17,17,17
};

struct Image
{
    size_t num_tracks;
    IBlockAllocator *allocator;
    FileMap *map;
    Track *tracks[IMAGE_NUM_TRACKS];
};

SOLOCAL size_t
Image_objectSize(void)
{
    return sizeof(Image);
}

SOLOCAL Image *
Image_init(Image *this)
{
    int i;

    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        this->tracks[i] = OBJNEW2(Track, i+1, num_sectors[i]);
    }
    this->num_tracks = IMAGE_NUM_TRACKS;
    this->allocator = &defaultAllocator;
    this->map = OBJNEW(FileMap);
    defaultAllocator.setImage(&defaultAllocator, this);

    return this;
}

SOLOCAL void
Image_done(Image *this)
{
    int i;

    for (i = 0; i < IMAGE_NUM_TRACKS; ++i)
    {
        OBJDEL(Track, this->tracks[i]);
    }
    OBJDEL(FileMap, this->map);
}

SOEXPORT BlockStatus
Image_blockStatus(const Image *this, const BlockPosition *pos)
{
    Track *t = Image_track(this, pos->track);
    if (!t) return (BlockStatus) -1;
    return Track_blockStatus(t, pos->sector);
}

SOEXPORT Track *
Image_track(const Image *this, int track)
{
    Track *t;

    if (track > 0 && track <= IMAGE_NUM_TRACKS)
    {
        t = this->tracks[track-1];
    }
    else
    {
        t = ModRepo_firstGetTrack(Mkd64_modRepo(MKD64), track);
    }

    return t;
}

SOEXPORT Block *
Image_block(const Image *this, const BlockPosition *pos)
{
    Track *t = Image_track(this, pos->track);
    if (!t) return 0;
    return Track_block(t, pos->sector);
}

SOEXPORT Block *
Image_allocateAt(const Image *this, const BlockPosition *pos)
{
    Block *b = Image_block(this, pos);
    BlockStatus s;

    if (!b) return 0;
    s = Block_status(b);
    if (s & BS_ALLOCATED) return 0;
    if (s & BS_RESERVED)
    {
        if (!Block_unReserve(b)) return 0;
    }
    Block_allocate(b);
    return b;
}

SOLOCAL FileMap *
Image_fileMap(const Image *this)
{
    return this->map;
}

SOEXPORT void
Image_setAllocator(Image *this, IBlockAllocator *allocator)
{
    allocator->setImage(allocator, this);
    this->allocator = allocator;
}

SOEXPORT IBlockAllocator *
Image_allocator(const Image *this)
{
    return this->allocator;
}

SOLOCAL int
Image_dump(const Image *this, FILE *out)
{
    int tracknum = 0;
    Track *track;
    Block *block;
    size_t num_sectors;
    int i;

    while ((track = Image_track(this, ++tracknum)))
    {
        num_sectors = Track_numSectors(track);
        for (i = 0; i < num_sectors; ++i)
        {
            block = Track_block(track, i);
            if (fwrite(Block_rawData(block), BLOCK_RAWSIZE, 1, out) != 1)
                return 0;
        }
    }
    return 1;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
