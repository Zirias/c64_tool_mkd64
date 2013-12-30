#ifndef MKD64_BLOCK_H
#define MKD64_BLOCK_H

#include "stdintrp.h"

#include "blckstat.h"

#define BLOCK_SIZE 254
#define BLOCK_RAWSIZE 256

struct block;
typedef struct block Block;

typedef struct
{
    uint8_t track;
    uint8_t sector;
} BlockPosition;

Block *block_new();
void block_delete(Block *this);

BlockStatus block_status(const Block *this);

uint8_t block_nextTrack(const Block *this);
uint8_t block_nextSector(const Block *this);
void block_nextPosition(const Block *this, BlockPosition *pos);

void block_setNextTrack(Block *this, uint8_t nextTrack);
void block_setNextSector(Block *this, uint8_t nextSector);
void block_setNextPosition(Block *this, const BlockPosition *pos);

int block_reserve(Block *this);
int block_allocate(Block *this);

uint8_t *block_data(Block *this);
uint8_t *block_rawData(Block *this);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
