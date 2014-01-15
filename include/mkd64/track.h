#ifndef MKD64_TRACK_H
#define MKD64_TRACK_H

#include <stdlib.h>

/** class representing a track on a C64 disk
 */
typedef struct Track Track;

#include <mkd64/block.h>
#include <mkd64/imodule.h>

#define TRACK_MAX_SECTORS 21

/** Get object size for a track
 * @return size of a track instance
 */
DECLEXPORT size_t Track_objectSize(void);

/** Initialize a track instance
 * @param this the track
 * @param tracknum the number of this track on the disk image
 * @param num_sectors the number of sectors to use on this track
 * @return a new track instance
 */
DECLEXPORT Track *Track_init(Track *this, int tracknum, size_t num_sectors);

/** Delete a track instance
 * This frees all memory allocated by this track and the blocks contained in it
 * @param this the track to delete
 */
DECLEXPORT void Track_done(Track *this);

/** Get the block status of the block in a given sector
 * @param this the track
 * @param sector the sector number
 * @return the block status
 */
DECLEXPORT BlockStatus Track_blockStatus(const Track *this, int sector);

/** Get number of sectors on this track
 * @param this the track
 * @return the number of sectors
 */
DECLEXPORT size_t Track_numSectors(const Track *this);

/** Get number of free sectors on this track
 * @param this the track
 * @param mask a mask of BlockStatus values to be considered "free" in addition
 *  to blocks with the "BS_NONE" status. E.g. specify "BS_RESERVED" here to
 *  include reserved blocks, that are not allocated yet.
 * @return the number of free sectors
 */
DECLEXPORT int Track_freeSectors(const Track *this, BlockStatus mask);

/** Reserve the block in a given sector
 * @param this the track
 * @param sector the sector number
 * @param by the module reserving the block
 * @return 1 on success, 0 otherwise
 */
DECLEXPORT int Track_reserveBlock(Track *this, int sector, IModule *by);

/** Allocate the block in a given sector
 * ATTENTION: This does not check anything. Make sure you never allocate a
 * block that is currently reserved by some other module. If you need a
 * reserved block, ask for it using block_unReserve()!
 * @param this the track
 * @param sector the sector number
 * @return 1 on success, 0 otherwise
 */ 
DECLEXPORT int Track_allocateBlock(Track *this, int sector);

/** Allocate first available block from a given start sector
 * This is a convenience function to find and allocate the next available
 * sector by searching sequentially from a given sector number, mostly
 * useful while implementing a custom IBlockAllocator.
 * @param this the track
 * @param sector the number of the sector to try first
 * @param considerReserved if 1, try unreserving any reserved but not yet
 *  allocated sector.
 * @return the sector number of the newly allocated block, or -1 if no
 *  available block was found.
 */
DECLEXPORT int Track_allocateFirstFreeFrom(Track *this,
        int sector, int considerReserved);

/** Get the block object at a given sector number
 * @param this the track
 * @param sector the sector number
 * @return the block object, or 0 if no block at the given sector number exists
 */
DECLEXPORT Block *Track_block(const Track *this, int sector);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
