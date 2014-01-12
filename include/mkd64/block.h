#ifndef MKD64_BLOCK_H
#define MKD64_BLOCK_H

#include <mkd64/common.h>

#define BLOCK_SIZE 254
#define BLOCK_RAWSIZE 256

struct block;
/** class representing a single block on a C64 disk
 */
typedef struct block Block;

typedef struct
{
    uint8_t track;
    uint8_t sector;
} BlockPosition;

typedef enum
{
    BS_NONE = 0,
    BS_ALLOCATED = 1,
    BS_RESERVED = 1 << 1
} BlockStatus;

#include <mkd64/imodule.h>

/** Get status of the block
 * @param this the block
 * @return the current status
 */
DECLEXPORT BlockStatus block_status(const Block *this);

/** Get position (track/sector) of the block
 * @param this the block
 * @return the position of the block
 */
DECLEXPORT const BlockPosition *block_position(const Block *this);

/** Get track number of the following block in the chain
 * returns the first byte of the block, which is the track number of the next
 * block for chained blocks.
 * @param this the block
 * @return track number of the next block
 */
DECLEXPORT uint8_t block_nextTrack(const Block *this);

/** Get sector number of the following block in the chain
 * returns the second byte of the block, which is the sector number of the next
 * block for chained blocks.
 * @param this the block
 * @return sector number of the next block
 */
DECLEXPORT uint8_t block_nextSector(const Block *this);

/** Get the position of the following block in the chain
 * The BlockPosition pos is filled with the first two bytes of the block,
 * which is the position of the next block for chained blocks.
 * @param this the block
 * @param pos the BlockPosition to hold the result
 */
DECLEXPORT void block_nextPosition(const Block *this, BlockPosition *pos);

/** Get module instance reserving the current block
 * @param this the block
 * @return pointer to the module reserving this block, or 0 if not reserved
 */
DECLEXPORT IModule *block_reservedBy(const Block *this);

/** Set track number of the following block in the chain
 * @param this the block
 * @param nextTrack track number of the next block in the chain
 */
DECLEXPORT void block_setNextTrack(Block *this, uint8_t nextTrack);

/** Set sector number of the following block in the chain
 * @param this the block
 * @param nextSector sector number of the next block in the chain
 */
DECLEXPORT void block_setNextSector(Block *this, uint8_t nextSector);

/** Set the position of the following block in the chain
 * @param this the block
 * @param pos the position of the next block in the chain
 */
DECLEXPORT void block_setNextPosition(Block *this, const BlockPosition *pos);

/** Reserve this block for later use
 * @param this the block
 * @param by pointer to the module reserving the block
 * @return 1 on success, 0 on failure
 */
DECLEXPORT int block_reserve(Block *this, IModule *by);

/** Try to unreserve the block by asking the module to release it
 * @param this the block
 * @return 1 on success, 0 on failure
 */
DECLEXPORT int block_unReserve(Block *this);

/** Allocate the current block
 * ATTENTION: This does not check anything. Make sure you never allocate a
 * block that is currently reserved by some other module. If you need a
 * reserved block, ask for it using block_unReserve()!
 * @param this the block
 * @return 1 on success, 0 on failure
 */
DECLEXPORT int block_allocate(Block *this);

/** Free the current block
 * ATTENTION: This does not check anything. Make sure you never free a
 * block you didn't allocate yourself!
 * @param this the block
 * @return 1 on success, 0 on failure
 */
DECLEXPORT int block_free(Block *this);

/** Get a pointer to the 254 bytes of data inside the block
 * ATTENTION: You can call this for ANY block. Make sure you never modify
 * block data allocated or reserved by some other module!
 * @param this the block
 * @return pointer to the data
 */
DECLEXPORT uint8_t *block_data(Block *this);


/** Get a pointer to the 256 bytes of raw data inside the block
 * This includes the first two bytes that are normally used for chaining.
 * ATTENTION: You can call this for ANY block. Make sure you never modify
 * block data allocated or reserved by some other module!
 * @param this the block
 * @return pointer to the data
 */
DECLEXPORT uint8_t *block_rawData(Block *this);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
