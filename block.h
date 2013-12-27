#ifndef MKD64_BLOCK_H
#define MKD64_BLOCK_H

#include <stdint.h>

struct block;
typedef struct block Block;

Block *block_new();
void block_delete(Block *this);

Block *block_newAt(int track, int sector);
Block *block_newFrom(uint8_t data[256]);
Block *block_newAtFrom(int track, int sector, uint8_t data[256]);

int block_track(const Block *this);
int block_sector(const Block *this);
uint8_t *block_data(const Block *this);

int block_setPosition(Block *this, int track, int sector);
int block_setData(Block *this, uint8_t data[256]);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
