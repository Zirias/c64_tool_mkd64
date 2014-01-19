#ifndef MKD64_BLOCK_H
#define MKD64_BLOCK_H

/** class Block.
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <mkd64/common.h>

#define BLOCK_SIZE 254      /**< the usable size of a block (254 bytes) */
#define BLOCK_RAWSIZE 256   /**< the raw size of a block (256 bytes) */

/** Class representing a single block on a C64 disk.
 * @class Block mkd64/block.h
 */
typedef struct Block Block;

/** A position on a disk image, given in track and sector number.
 * @struct BlockPosition mkd64/block.h
 */
typedef struct BlockPosition BlockPosition;
struct BlockPosition
{
    uint8_t track;  /**< track number, starting at 1 */
    uint8_t sector; /**< sector number, starting at 0 */
};

/** The current status of a block on a C64 disk.
 * @enum BlockStatus mkd64/block.h
 */
typedef enum
{
    BS_NONE = 0,            /**< block is available */
    BS_ALLOCATED = 1,       /**< block is allocated */
    BS_RESERVED = 1 << 1    /**< block is reserved by a module */
} BlockStatus;

#include <mkd64/imodule.h>

/** Get status of the block.
 * @relates Block
 * @param self the block
 * @return the current status
 */
DECLEXPORT BlockStatus Block_status(const Block *self);

/** Get position (track/sector) of the block.
 * @relates Block
 * @param self the block
 * @return the position of the block
 */
DECLEXPORT const BlockPosition *Block_position(const Block *self);

/** Get track number of the following block in the chain.
 * returns the first byte of the block, which is the track number of the next
 * block for chained blocks.
 * @relates Block
 * @param self the block
 * @return track number of the next block
 */
DECLEXPORT uint8_t Block_nextTrack(const Block *self);

/** Get sector number of the following block in the chain.
 * returns the second byte of the block, which is the sector number of the next
 * block for chained blocks.
 * @relates Block
 * @param self the block
 * @return sector number of the next block
 */
DECLEXPORT uint8_t Block_nextSector(const Block *self);

/** Get the position of the following block in the chain.
 * The BlockPosition pos is filled with the first two bytes of the block,
 * which is the position of the next block for chained blocks.
 * @relates Block
 * @param self the block
 * @param pos the BlockPosition to hold the result
 */
DECLEXPORT void Block_nextPosition(const Block *self, BlockPosition *pos);

/** Get module instance reserving the current block.
 * @relates Block
 * @param self the block
 * @return pointer to the module reserving this block, or 0 if not reserved
 */
DECLEXPORT IModule *Block_reservedBy(const Block *self);

/** Set track number of the following block in the chain.
 * @relates Block
 * @param self the block
 * @param nextTrack track number of the next block in the chain
 */
DECLEXPORT void Block_setNextTrack(Block *self, uint8_t nextTrack);

/** Set sector number of the following block in the chain.
 * @relates Block
 * @param self the block
 * @param nextSector sector number of the next block in the chain
 */
DECLEXPORT void Block_setNextSector(Block *self, uint8_t nextSector);

/** Set the position of the following block in the chain.
 * @relates Block
 * @param self the block
 * @param pos the position of the next block in the chain
 */
DECLEXPORT void Block_setNextPosition(Block *self, const BlockPosition *pos);

/** Reserve this block for later use.
 * @relates Block
 * @param self the block
 * @param by pointer to the module reserving the block
 * @return 1 on success, 0 on failure
 */
DECLEXPORT int Block_reserve(Block *self, IModule *by);

/** Try to unreserve the block by asking the module to release it.
 * @relates Block
 * @param self the block
 * @return 1 on success, 0 on failure
 */
DECLEXPORT int Block_unReserve(Block *self);

/** Allocate the current block.
 * ATTENTION: This does not check anything. Make sure you never allocate a
 * block that is currently reserved by some other module. If you need a
 * reserved block, ask for it using Block_unReserve()!
 * @relates Block
 * @param self the block
 * @return 1 on success, 0 on failure
 */
DECLEXPORT int Block_allocate(Block *self);

/** Free the current block.
 * ATTENTION: This does not check anything. Make sure you never free a
 * block you didn't allocate yourself!
 * @relates Block
 * @param self the block
 * @return 1 on success, 0 on failure
 */
DECLEXPORT int Block_free(Block *self);

/** Get a pointer to the 254 bytes of data inside the block.
 * ATTENTION: You can call this for ANY block. Make sure you never modify
 * block data allocated or reserved by some other module!
 * @relates Block
 * @param self the block
 * @return pointer to the data
 */
DECLEXPORT uint8_t *Block_data(Block *self);


/** Get a pointer to the 256 bytes of raw data inside the block.
 * This includes the first two bytes that are normally used for chaining.
 * ATTENTION: You can call this for ANY block. Make sure you never modify
 * block data allocated or reserved by some other module!
 * @relates Block
 * @param self the block
 * @return pointer to the data
 */
DECLEXPORT uint8_t *Block_rawData(Block *self);

#ifdef __cplusplus
}
#endif

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
