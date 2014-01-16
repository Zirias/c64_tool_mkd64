#ifndef MKD64_IBLALLOC_H
#define MKD64_IBLALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

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
     * self is always called when the allocator is attached to an image, so
     * do all your initialization here.
     * @param self the IBlockAllocator
     * @param image the image to work on
     */
    void (*setImage)(IBlockAllocator *self, Image *image);

    /** Set the requested interleave for all following operations
     * @param self the IBlockAllocator
     * @param interleave the requested interleave value
     */
    void (*setInterleave)(IBlockAllocator *self, int interleave);

    /** Set whether the allocator should consider allocating reserved blocks
     * If self was called with a 1, the allocator should try to get hold of
     * reserved blocks and allocate them if possible.
     * @param self the IBlockAllocator
     * @param considerReserved 1 or 0, 1 means try to allocate reserved blocks
     */
    void (*setConsiderReserved)(IBlockAllocator *self, int considerReserved);

    /** Try allocating a first block for a new file
     * self should locate a free block, allocate it and return it.
     * @param self the IBlockAllocator
     * @return a newly-allocated block or 0 if no available block was found
     */
    Block *(*allocFirstBlock)(IBlockAllocator *self);

    /** Try allocating a new block for a file
     * self should locate the best available block for chaining to the
     * position given in pos, honouring the current interleave value. It should
     * NOT actively chain the blocks, self is left to the caller -- just find
     * a suitable block, allocate it and return it.
     * @param self the IBlockAllocator
     * @param pos the position of the previous block in the chain
     * @return a newly-allocated block or 0 if no available block was found
     */
    Block *(*allocNextBlock)(IBlockAllocator *self, const BlockPosition *pos);
};

#ifdef __cplusplus
}
#endif

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
