#ifndef MKD64_IBLALLOC_H
#define MKD64_IBLALLOC_H

#include <mkd64/common.h>

/** interface for custom block allocation schemes
 */
typedef struct IBlockAllocator IBlockAllocator;

#include <mkd64/image.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

struct IBlockAllocator
{
    /** Set the image the allocator should operate on
     * This is always called when the allocator is attached to an image, so
     * do all your initialization here.
     * @param this the IBlockAllocator
     * @param image the image to work on
     */
    void (*setImage)(IBlockAllocator *this, Image *image);

    /** Set the requested interleave for all following operations
     * @param this the IBlockAllocator
     * @param interleave the requested interleave value
     */
    void (*setInterleave)(IBlockAllocator *this, int interleave);

    /** Set whether the allocator should consider allocating reserved blocks
     * If this was called with a 1, the allocator should try to get hold of
     * reserved blocks and allocate them if possible.
     * @param this the IBlockAllocator
     * @param considerReserved 1 or 0, 1 means try to allocate reserved blocks
     */
    void (*setConsiderReserved)(IBlockAllocator *this, int considerReserved);

    /** Try allocating a first block for a new file
     * This should locate a free block, allocate it and return it.
     * @param this the IBlockAllocator
     * @return a newly-allocated block or 0 if no available block was found
     */
    Block *(*allocFirstBlock)(IBlockAllocator *this);

    /** Try allocating a new block for a file
     * This should locate the best available block for chaining to the
     * position given in pos, honouring the current interleave value. It should
     * NOT actively chain the blocks, this is left to the caller -- just find
     * a suitable block, allocate it and return it.
     * @param this the IBlockAllocator
     * @param pos the position of the previous block in the chain
     * @return a newly-allocated block or 0 if no available block was found
     */
    Block *(*allocNextBlock)(IBlockAllocator *this, const BlockPosition *pos);
};

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
