#ifndef CBMDOS_ALLOC_H
#define CBMDOS_ALLOC_H

struct cbmdosAllocator;
typedef struct cbmdosAllocator CbmdosAllocator;

#include <mkd64/iblalloc.h>

IBlockAllocator *cbmdosAllocator_new(void);
void cbmdosAllocator_delete(IBlockAllocator *this);

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
