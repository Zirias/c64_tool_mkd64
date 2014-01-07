#ifndef MKD64_IMAGE_H
#define MKD64_IMAGE_H

#include <mkd64/common.h>

struct image;
typedef struct image Image;

struct iBlockAllocator;
typedef struct iBlockAllocator IBlockAllocator;

#include <mkd64/image.h>
#include <mkd64/block.h>
#include <mkd64/track.h>

struct iBlockAllocator
{
    /** Allocate the next available block for a disk file
     * Implementations must consider the position given in pos first, and
     * only if this block is not available, search for an available block
     * according to the given interleave value.
     *
     * @param this the IBlockAllocator object
     * @param image the disk image to operate on
     * @param interleave interleave value as requested for the file to write
     * @param pos the starting position. If an available block is found, pos
     *            must cotain the found block, otherwise it must stay unchanged.
     *            If pos->track is 0, the allocator must determine a starting
     *            block for a new file.
     * @param considerReserved if this is 1, consider reserved blocks and ask
     *                         the modules to release them. See track.h for
     *                         how to do this
     * @return 1 if a block was found and allocated, 0 otherwise
     */
    int (*nextFileBlock)(IBlockAllocator *this, Image *image,
            int interleave, BlockPosition *pos, int considerReserved);
};

DECLEXPORT BlockStatus image_blockStatus(const Image *this,
        const BlockPosition *pos);

DECLEXPORT Track *image_track(const Image *this, int tracknum);
DECLEXPORT Block *image_block(const Image *this, BlockPosition *pos);

DECLEXPORT void image_setAllocator(Image *this, IBlockAllocator *allocator);
DECLEXPORT int image_nextFileBlock(Image *this,
        int interleave, BlockPosition *pos);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
