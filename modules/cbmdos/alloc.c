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
setImage(IBlockAllocator *self, Image *image)
{
    CbmdosAllocator *a = (CbmdosAllocator *)self;
    a->img = image;
    a->ilv = 1;
    a->rsv = 0;
    a->msk = BS_NONE;
}

static void
setInterleave(IBlockAllocator *self, int interleave)
{
    CbmdosAllocator *a = (CbmdosAllocator *)self;
    a->ilv = interleave;
}

static void
setConsiderReserved(IBlockAllocator *self, int considerReserved)
{
    CbmdosAllocator *a = (CbmdosAllocator *)self;
    a->rsv = considerReserved;
    a->msk = considerReserved ? BS_RESERVED : BS_NONE;
}

static Block *
allocFirstBlock(IBlockAllocator *self)
{
    CbmdosAllocator *a = (CbmdosAllocator *)self;
    Track *t;
    Block *b;
    int tn, td, sn;

    /* find first track that has free sectors left */
    td = 0;
    tn = 18;
    t = Image_track(a->img, tn);
    while (t)
    {
        if (Track_freeSectors(t, a->msk)) break;
        ++td;
        tn = 18 - td;
        if (tn > 0)
        {
            t = Image_track(a->img, tn);
            if (Track_freeSectors(t, a->msk)) break;
        }
        tn = 18 + td;
        t = Image_track(a->img, tn);
    }

    /* no track found */
    if (!t) return 0;

    /* allocate first available sector on block */
    sn = Track_allocateFirstFreeFrom(t, 0, a->rsv);
    b = Track_block(t, sn);
    return b;
}

static Block *
allocNextBlock(IBlockAllocator *self, const BlockPosition *pos)
{
    CbmdosAllocator *a = (CbmdosAllocator *)self;
    Track *t;
    Block *b;
    int tn, sn, half, bigdist;

    tn = pos->track;
    sn = pos->sector;

    /* get current track */
    t = Image_track(a->img, tn);
    half = (tn <= 18) ? 0 : 1;
    bigdist = 0;

    /* search first track from current one with free sectors */
    while (t && !Track_freeSectors(t, a->msk))
        t = Image_track(a->img, half ? ++tn : --tn);

    /* try again in the other half of the disk */
    if (!t)
    {
        half = !half;
        bigdist = 1;
        tn = half ? 19 : 18;
        t = Image_track(a->img, tn);
        while (t && !Track_freeSectors(t, a->msk))
            t = Image_track(a->img, half ? ++tn : --tn);
    }

    /* try again in the WHOLE initial half of the disk */
    if (!t)
    {
        half = !half;
        tn = half ? 19 : 18;
        t = Image_track(a->img, tn);
        while (t && !Track_freeSectors(t, a->msk))
            t = Image_track(a->img, half ? ++tn : --tn);
    }

    /* no track found, give up */
    if (!t) return 0;

    /* apply interleaving for "near" tracks */
    if (!bigdist) sn = (sn + a->ilv) % Track_numSectors(t);

    sn = Track_allocateFirstFreeFrom(t, sn, a->rsv);
    b = Track_block(t, sn);
    return b;
}

SOLOCAL IBlockAllocator *
cbmdosAllocator_new(void)
{
    IBlockAllocator *self = malloc(sizeof(CbmdosAllocator));

    self->setImage = &setImage;
    self->setInterleave = &setInterleave;
    self->setConsiderReserved = &setConsiderReserved;
    self->allocFirstBlock = &allocFirstBlock;
    self->allocNextBlock = &allocNextBlock;

    return self;
}

SOLOCAL void
cbmdosAllocator_delete(IBlockAllocator *self)
{
    free(self);
}

/* vim: et:si:ts=8:sts=4:sw=4
*/
