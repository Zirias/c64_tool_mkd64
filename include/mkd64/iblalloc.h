#ifndef MKD64_IBLALLOC_H
#define MKD64_IBLALLOC_H

#include <mkd64/common.h>

struct iBlockAllocator;
typedef struct iBlockAllocator IBlockAllocator;

#include <mkd64/image.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

struct iBlockAllocator
{
    void (*setImage)(IBlockAllocator *this, Image *image);
    void (*setInterleave)(IBlockAllocator *this, int interleave);
    void (*setConsiderReserved)(IBlockAllocator *this, int considerReserved);
    Block *(*allocFirstBlock)(IBlockAllocator *this);
    Block *(*allocNextBlock)(IBlockAllocator *this, const BlockPosition *pos);
};

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
