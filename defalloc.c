#include <mkd64/common.h>
#include <mkd64/iblalloc.h>
#include <mkd64/image.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

static Image *_img;
static int _ilv;
static int _rsv;
static BlockStatus _msk;

static void setImage(IBlockAllocator *self, Image *image);
static void setInterleave(IBlockAllocator *self, int interleave);
static void setConsiderReserved(IBlockAllocator *self, int considerReserved);
static Block *allocFirstBlock(IBlockAllocator *self);
static Block *allocNextBlock(IBlockAllocator *self, const BlockPosition *pos);

IBlockAllocator defaultAllocator = {
    &setImage,
    &setInterleave,
    &setConsiderReserved,
    &allocFirstBlock,
    &allocNextBlock
};

static void
setImage(IBlockAllocator *self, Image *image)
{
    _img = image;
    _ilv = 1;
    _rsv = 0;
    _msk = BS_NONE;
}

static void
setInterleave(IBlockAllocator *self, int interleave)
{
    _ilv = interleave;
}

static void
setConsiderReserved(IBlockAllocator *self, int considerReserved)
{
    _rsv = considerReserved;
    _msk = _rsv ? BS_RESERVED : BS_NONE;
}

static Block *
allocFirstBlock(IBlockAllocator *self)
{
    Track *t;
    Block *b;
    int tn, sn;

    /* find first track that has free sectors left */
    for (tn = 1, t = Image_track(_img, tn);
            t && !Track_freeSectors(t, _msk);
            t = Image_track(_img, ++tn)) {}

    /* no track found */
    if (!t) return 0;

    /* allocate first available sector on block */
    sn = Track_allocateFirstFreeFrom(t, 0, _rsv);
    b = Track_block(t, sn);
    return b;
}

static Block *
allocNextBlock(IBlockAllocator *self, const BlockPosition *pos)
{
    Track *t;
    Block *b;
    int tn, sn, bigdist;

    tn = pos->track;
    sn = pos->sector;

    /* get current track */
    t = Image_track(_img, tn);
    bigdist = 0;

    /* search first track from current one with free sectors */
    while (t && !Track_freeSectors(t, _msk)) t = Image_track(_img, ++tn);

    /* try again from beginning of the disk if no track found so far */
    if (!t)
    {
        bigdist = 1;
        tn = 1;
        t = Image_track(_img, tn);
        while (tn < pos->track && !Track_freeSectors(t, _msk))
            t = Image_track(_img, ++tn);
    }

    /* no track found */
    if (!t) return 0;

    /* apply interleaving for "near" tracks */
    if (!bigdist) sn = (sn + _ilv) % Track_numSectors(t);

    sn = Track_allocateFirstFreeFrom(t, sn, _rsv);
    b = Track_block(t, sn);
    return b;
}

/* vim: et:si:ts=8:sts=4:sw=4
*/
