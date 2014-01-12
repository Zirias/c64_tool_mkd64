#ifndef MKD64_IMAGE_H
#define MKD64_IMAGE_H

#include <mkd64/common.h>

struct image;
/** class representing a D64 image in memory
 */
typedef struct image Image;

#include <mkd64/image.h>
#include <mkd64/block.h>
#include <mkd64/track.h>
#include <mkd64/iblalloc.h>

/** Get the current status of a block at a given position
 * @param this the image
 * @param pos the position of the block
 * @return the status of the block
 */
DECLEXPORT BlockStatus image_blockStatus(const Image *this,
        const BlockPosition *pos);

/** Get track object for a given track number
 * This also tries to get a track from modules providing extra tracks
 * @param this the image
 * @param tracknum the number of the track to get
 * @return the track object or 0 if no track with that number exists
 */
DECLEXPORT Track *image_track(const Image *this, int tracknum);

/** Get block object for a given position
 * This also locates blocks on tracks from modules providing extra tracks
 * @param this the image
 * @param pos the position of the block to get
 * @return the block object or 0 if no block wit the given position exists
 */
DECLEXPORT Block *image_block(const Image *this, const BlockPosition *pos);

/** Try to allocate a block at a fixed position
 * This tries to allocate a given block. A module reserving this block is asked
 * to release it if necessary. It will fail if the block is already allocated
 * or a module reserving it refuses to release it.
 * @param this the image
 * @param pos the position of the block to allocate
 * @return the allocated block object or 0 if allocation was not possible
 */
DECLEXPORT Block *image_allocateAt(const Image *this, const BlockPosition *pos);

/** Set custom allocator
 * Set an IBlockAllocator instance to handle block allocations while writing
 * files. The Image class has a default allocator that is used if this is never
 * called -- the default allocator is looking sequentially for free blocks,
 * starting at track #1.
 * @param this the image
 * @param allocator the allocator to use
 */
DECLEXPORT void image_setAllocator(Image *this, IBlockAllocator *allocator);

/** Get current allocator
 * Get a pointer to the IBlockAllocator instance currently in use.
 * @param this the image
 * @return the allocator
 */
DECLEXPORT IBlockAllocator *image_allocator(const Image *this);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
