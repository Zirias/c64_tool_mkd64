#ifndef MKD64_BLCKSTAT_H
#define MKD64_BLCKSTAT_H

typedef enum
{
    BS_NONE = 0,
    BS_ALLOCATED = 1,
    BS_RESERVED = 1 << 1,
} BlockStatus;

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
