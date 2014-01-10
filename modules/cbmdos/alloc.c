#include <mkd64/common.h>
#include <mkd64/iblalloc.h>
#include <mkd64/image.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

#include "alloc.h"

#include <stdlib.h>

struct cbmdosAllocator
{
    IBlockAllocator iba;
    Image *img;
    int ilv;
    int rsv;
    BlockStatus msk;
};

static void
setImage(IBlockAllocator *this, Image *image)
{
    CbmdosAllocator *a = (CbmdosAllocator *)this;
    a->img = image;
    a->ilv = 1;
    a->rsv = 0;
    a->msk = BS_NONE;
}

static void
setInterleave(IBlockAllocator *this, int interleave)
{
    CbmdosAllocator *a = (CbmdosAllocator *)this;
    a->ilv = interleave;
}

static void
setConsiderReserved(IBlockAllocator *this, int considerReserved)
{
    CbmdosAllocator *a = (CbmdosAllocator *)this;
    a->rsv = considerReserved;
    a->msk = considerReserved ? BS_RESERVED : BS_NONE;
}

static Block *
allocFirstBlock(IBlockAllocator *this, const BlockPosition *pos)
{
    CbmdosAllocator *a = (CbmdosAllocator *)this;
    Track *t;
    Block *b;
    int tn, td, sn;

    if (pos && pos->track > 0)
    {
        /* fixed start position requested */
        b = image_block(a->img, pos);
        if (block_status(b) & ~a->msk) return 0;
        return b;
    }

    /* find first track that has free sectors left */
    td = 0;
    tn = 18;
    t = image_track(a->img, tn);
    while (t)
    {
        if (track_freeSectors(t, a->msk)) break;
        ++td;
        tn = 18 - td;
        if (tn > 0)
        {
            t = image_track(a->img, tn);
            if (track_freeSectors(t, a->msk)) break;
        }
        tn = 18 + td;
        t = image_track(a->img, tn);
    }

    /* no track found */
    if (!t) return 0;

    /* allocate first available sector on block */
    sn = track_allocateFirstFreeFrom(t, 0, a->rsv);
    b = track_block(t, sn);
    return b;
}

static Block *
allocNextBlock(IBlockAllocator *this, const BlockPosition *pos)
{
    CbmdosAllocator *a = (CbmdosAllocator *)this;
    Track *t;
    Block *b;
    int tn, sn, half, bigdist;

    tn = pos->track;
    sn = pos->sector;

    /* get current track */
    t = image_track(a->img, tn);
    half = (tn <= 18) ? 0 : 1;
    bigdist = 0;

    /* search first track from current one with free sectors */
    while (t && !track_freeSectors(t, a->msk))
        t = image_track(a->img, half ? ++tn : --tn);

    /* try again in the other half of the disk */
    if (!t)
    {
        half = !half;
        bigdist = 1;
        tn = half ? 19 : 18;
        t = image_track(a->img, tn);
        while (t && !track_freeSectors(t, a->msk))
            t = image_track(a->img, half ? ++tn : --tn);
    }

    /* try again in the WHOLE initial half of the disk */
    if (!t)
    {
        half = !half;
        tn = half ? 19 : 18;
        t = image_track(a->img, tn);
        while (t && !track_freeSectors(t, a->msk))
            t = image_track(a->img, half ? ++tn : --tn);
    }

    /* no track found, give up */
    if (!t) return 0;

    /* apply interleaving for "near" tracks */
    if (!bigdist) sn = (sn + a->ilv) % track_numSectors(t);

    sn = track_allocateFirstFreeFrom(t, sn, a->rsv);
    b = track_block(t, sn);
    return b;
}

SOLOCAL IBlockAllocator *
cbmdosAllocator_new(void)
{
    IBlockAllocator *this = malloc(sizeof(CbmdosAllocator));

    this->setImage = &setImage;
    this->setInterleave = &setInterleave;
    this->setConsiderReserved = &setConsiderReserved;
    this->allocFirstBlock = &allocFirstBlock;
    this->allocNextBlock = &allocNextBlock;

    return this;
}

SOLOCAL void
cbmdosAllocator_delete(IBlockAllocator *this)
{
    free(this);
}

/* vim: et:si:ts=8:sts=4:sw=4
*/
