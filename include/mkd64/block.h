#ifndef MKD64_BLOCK_H
#define MKD64_BLOCK_H

#include <mkd64/common.h>

#define BLOCK_SIZE 254
#define BLOCK_RAWSIZE 256

struct block;
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
    BS_RESERVED = 1 << 1,
} BlockStatus;

DECLEXPORT BlockStatus block_status(const Block *this);
DECLEXPORT const BlockPosition *block_position(const Block *this);

DECLEXPORT uint8_t block_nextTrack(const Block *this);
DECLEXPORT uint8_t block_nextSector(const Block *this);
DECLEXPORT void block_nextPosition(const Block *this, BlockPosition *pos);

DECLEXPORT void block_setNextTrack(Block *this, uint8_t nextTrack);
DECLEXPORT void block_setNextSector(Block *this, uint8_t nextSector);
DECLEXPORT void block_setNextPosition(Block *this, const BlockPosition *pos);

DECLEXPORT int block_reserve(Block *this);
DECLEXPORT int block_unReserve(Block *this);
DECLEXPORT int block_allocate(Block *this);
DECLEXPORT int block_free(Block *this);

DECLEXPORT uint8_t *block_data(Block *this);
DECLEXPORT uint8_t *block_rawData(Block *this);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
