#ifndef MKD64_IMAGE_H
#define MKD64_IMAGE_H

/** class Image.
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <mkd64/common.h>

/** Class representing a D64 image in memory.
 * @class Image mkd64/image.h
 */
typedef struct Image Image;

#include <mkd64/image.h>
#include <mkd64/block.h>
#include <mkd64/track.h>
#include <mkd64/iblalloc.h>

/** Get the current status of a block at a given position.
 * @relates Image
 * @param self the image
 * @param pos the position of the block
 * @return the status of the block
 */
DECLEXPORT BlockStatus Image_blockStatus(const Image *self,
        const BlockPosition *pos);

/** Get track object for a given track number.
 * This also tries to get a track from modules providing extra tracks
 * @relates Image
 * @param self the image
 * @param tracknum the number of the track to get
 * @return the track object or 0 if no track with that number exists
 */
DECLEXPORT Track *Image_track(const Image *self, int tracknum);

/** Get block object for a given position.
 * This also locates blocks on tracks from modules providing extra tracks
 * @relates Image
 * @param self the image
 * @param pos the position of the block to get
 * @return the block object or 0 if no block wit the given position exists
 */
DECLEXPORT Block *Image_block(const Image *self, const BlockPosition *pos);

/** Try to allocate a block at a fixed position.
 * This tries to allocate a given block. A module reserving this block is asked
 * to release it if necessary. It will fail if the block is already allocated
 * or a module reserving it refuses to release it.
 * @relates Image
 * @param self the image
 * @param pos the position of the block to allocate
 * @return the allocated block object or 0 if allocation was not possible
 */
DECLEXPORT Block *Image_allocateAt(const Image *self, const BlockPosition *pos);

/** Set custom allocator.
 * Set an IBlockAllocator instance to handle block allocations while writing
 * files. The Image class has a default allocator that is used if this is never
 * called -- the default allocator is looking sequentially for free blocks,
 * starting at track #1.
 * @relates Image
 * @param self the image
 * @param allocator the allocator to use
 */
DECLEXPORT void Image_setAllocator(Image *self, IBlockAllocator *allocator);

/** Get current allocator.
 * Get a pointer to the IBlockAllocator instance currently in use.
 * @relates Image
 * @param self the image
 * @return the allocator
 */
DECLEXPORT IBlockAllocator *Image_allocator(const Image *self);

#ifdef __cplusplus
}
#endif

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
