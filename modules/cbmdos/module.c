#include <mkd64/common.h>
#include <mkd64/mkd64.h>
#include <mkd64/imodule.h>
#include <mkd64/debug.h>
#include <mkd64/block.h>
#include <mkd64/track.h>
#include <mkd64/util.h>

#include <stdio.h>
#include <string.h>

#include "buildid.h"
#include "alloc.h"

MKD64_MODULE("cbmdos")

#define MODVERSION "1.3b"

struct dirBlock;
typedef struct dirBlock DirBlock;

struct dirBlock
{
    DirBlock *next;
    BlockPosition pos;
};

typedef enum
{
    CDFL_NONE = 0,
    CDFL_SLOTRESERVED = 1 << 0,
    CDFL_DIROVERFLOW = 1 << 1,
    CDFL_ALLOCALL = 1 << 2,
    CDFL_ZEROFREE = 1 << 3
} CbmdosFlags;

typedef struct
{
    IModule mod;
    Image *image;
    Block *bam;
    DirBlock *directory;
    CbmdosFlags flags;
    int reservedDirBlocks;
    int dirInterleave;
    int usedDirBlocks;
    Block *currentDirBlock;
    int currentDirSlot;
    int extraDirBlocks;
    int reclaimedDirBlocks;
    IBlockAllocator *alloc;
} Cbmdos;

typedef enum
{
    FT_DEL = 0x80,
    FT_SEQ = 0x81,
    FT_PRG = 0x82,
    FT_USR = 0x83,
    FT_REL = 0x84,

    FT_PROT = 0x40
} CbmdosFileType;

typedef struct
{
    CbmdosFileType fileType;
    int writeDirEntry;
    int forceBlocks;
} CbmdosFileData;

/* allow only one instance */
static Cbmdos *singleInstance = 0;

static const uint8_t _initialBam[256] = {
    0x12, 0x01, 0x41, 0x00, 0x15, 0xff, 0xff, 0x1f,
    0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
    0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
    0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
    0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
    0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
    0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
    0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
    0x15, 0xff, 0xff, 0x1f, 0x15, 0xff, 0xff, 0x1f,
    0x13, 0xff, 0xff, 0x07, 0x13, 0xff, 0xff, 0x07,
    0x13, 0xff, 0xff, 0x07, 0x13, 0xff, 0xff, 0x07,
    0x13, 0xff, 0xff, 0x07, 0x13, 0xff, 0xff, 0x07,
    0x13, 0xff, 0xff, 0x07, 0x12, 0xff, 0xff, 0x03,
    0x12, 0xff, 0xff, 0x03, 0x12, 0xff, 0xff, 0x03,
    0x12, 0xff, 0xff, 0x03, 0x12, 0xff, 0xff, 0x03,
    0x12, 0xff, 0xff, 0x03, 0x11, 0xff, 0xff, 0x01,
    0x11, 0xff, 0xff, 0x01, 0x11, 0xff, 0xff, 0x01,
    0x11, 0xff, 0xff, 0x01, 0x11, 0xff, 0xff, 0x01,
    0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0,
    0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0xa0,
    0xa0, 0xa0, 0xa0, 0xa0, 0xa0, 0x32, 0x41, 0xa0,
    0xa0, 0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void
_deleteFileData(const void *owner, void *data)
{
    (void) owner; /* unused */

    free(data);
}

static int
nextDirBlock(Cbmdos *self, BlockPosition *pos, int allocate)
{
    Track *t;
    int trackChanged = 0;

    if (pos->track == 0)
    {
        pos->track = 18;
        pos->sector = 1;
    }

    t = Image_track(self->image, pos->track);
    while (t && !Track_freeSectors(t, BS_NONE))
    {
        trackChanged = 1;
        ++(pos->track);
        t = Image_track(self->image, pos->track);
    }
    if (!t)
    {
        trackChanged = 1;
        pos->track = 17;
        t = Image_track(self->image, pos->track);
        while (t && !Track_freeSectors(t, BS_NONE))
        {
            --(pos->track);
            t = Image_track(self->image, pos->track);
        }
    }
    if (!t) return 0;

    if (!trackChanged && Track_blockStatus(t, pos->sector) == BS_NONE)
    {
        if (allocate) Track_allocateBlock(t, pos->sector);
        else Track_reserveBlock(t, pos->sector, (IModule *)self);
        return 1;
    }

    pos->sector = (pos->sector + self->dirInterleave) % Track_numSectors(t);

    while (Track_blockStatus(t, pos->sector) != BS_NONE)
    {
        if (++(pos->sector) >= Track_numSectors(t)) pos->sector = 0;
    }

    if (allocate) Track_allocateBlock(t, pos->sector);
    else Track_reserveBlock(t, pos->sector, (IModule *)self);
    return 1;
}

static void
_reserveDirBlocks(Cbmdos *self)
{
    BlockPosition pos = {0,0};
    DirBlock *current;
    DirBlock *parent = 0;
    int i;

    for (i = 0; i < self->reservedDirBlocks; ++i)
    {
        if (!nextDirBlock(self, &pos, 0)) return;
        current = malloc(sizeof(DirBlock));
        current->next = 0;
        current->pos.track = pos.track;
        current->pos.sector = pos.sector;
        if (parent) parent->next = current;
        else self->directory = current;
        parent = current;
    }
}

static void
reserveDirSlot(Cbmdos *self)
{
    Block *nextBlock;
    BlockPosition pos;
    DirBlock *tmp;

    if (self->flags & CDFL_SLOTRESERVED) return;

    ++(self->currentDirSlot);
    if (self->currentDirSlot > 7)
    {
        if (self->directory)
        {
            nextBlock = Image_block(self->image, &(self->directory->pos));
            Block_allocate(nextBlock);
            tmp = self->directory;
            self->directory = tmp->next;
            free(tmp);
        }
        else
        {
            if (self->currentDirBlock)
            {
                pos.track = Block_position(self->currentDirBlock)->track;
                pos.sector = Block_position(self->currentDirBlock)->sector;
            }
            else
            {
                pos.track = 0;
                pos.sector = 0;
            }
            if (nextDirBlock(self, &pos, 1))
            {
                DBGd2("cbmdos: allocated extra directory block",
                        pos.track, pos.sector);
                nextBlock = Image_block(self->image, &pos);
                ++(self->extraDirBlocks);
            }
            else
            {
                fputs("[cbmdos] ERROR: no space left for directory!\n", stderr);
                --(self->currentDirSlot);
                self->flags |= CDFL_DIROVERFLOW;
                return;
            }
        }
        if (self->currentDirBlock)
        {
            Block_setNextTrack(self->currentDirBlock,
                    Block_position(nextBlock)->track);
            Block_setNextSector(self->currentDirBlock,
                    Block_position(nextBlock)->sector);
        }
        self->currentDirBlock = nextBlock;
        memset(Block_rawData(self->currentDirBlock), 0, 256);
        self->currentDirSlot = 0;
        ++(self->usedDirBlocks);
    }
    self->flags |= CDFL_SLOTRESERVED;
}

static void
_setDosVersion(Cbmdos *self, uint8_t version)
{
    Block_rawData(self->bam)[2] = version;
}

static void
_allocateAll(Cbmdos *self)
{
    self->flags |= CDFL_ALLOCALL;
    memset(Block_rawData(self->bam)+4, 0, 0x8c);
}

static void
_setZeroFree(Cbmdos *self)
{
    uint8_t *bamData;
    int i;

    if (!(self->flags & CDFL_ALLOCALL))
    {
        bamData = Block_rawData(self->bam);
        for (i = 0x4; i < 0x90; i += 0x4) bamData[i] = 0;
    }
    self->flags |= CDFL_ZEROFREE;
}

static void
delete(IModule *self)
{
    Cbmdos *dos = (Cbmdos *)self;
    DirBlock *tmp;

    while (dos->directory)
    {
        tmp = dos->directory;
        dos->directory = tmp->next;
        free(tmp);
    }

    cbmdosAllocator_delete(dos->alloc);

    free(self);
}

static void
initImage(IModule *self, Image *image)
{
    Cbmdos *dos = (Cbmdos *)self;
    BlockPosition pos = { 18, 0 };
    uint8_t *data;

    dos->image = image;
    dos->bam = Image_block(image, &pos);

    data = Block_rawData(dos->bam);
    memcpy(data, _initialBam, 256);

    /* write random disk id */
    data[0xa2] = randomNum(0x30, 0x53);
    if (data[0xa2] > 0x39) data[0xa2] += 7;
    data[0xa3] = randomNum(0x30, 0x53);
    if (data[0xa3] > 0x39) data[0xa3] += 7;

    Block_allocate(dos->bam);

    Image_setAllocator(image, dos->alloc);
}

static int
globalOption(IModule *self, char opt, const char *arg)
{
    Cbmdos *dos = (Cbmdos *)self;
    int intarg, arglen;
    unsigned int uintarg;

    switch (opt)
    {
        case 'd':
            if (checkArgAndWarn(opt, arg, 0, 1, id()))
            {
                arglen = strlen(arg);
                if (arglen > 16) arglen = 16;
                memcpy(Block_rawData(dos->bam) + 0x90, arg, arglen);
            }
            return 1;
        case 'i':
            if (checkArgAndWarn(opt, arg, 0, 1, id()))
            {
                arglen = strlen(arg);
                if (arglen > 5) arglen = 5;
                memcpy(Block_rawData(dos->bam) + 0xa2, arg, arglen);
            }
            return 1;
        case 'R':
            if (checkArgAndWarn(opt, arg, 0, 1, id()))
            {
                if (tryParseInt(arg, &intarg) && intarg >= 0)
                {
                    dos->reservedDirBlocks = intarg;
                }
                else
                {
                    fprintf(stderr, "[cbmdos] Warning: invalid reserved "
                            "blocks count `%s' ignored.\n", arg);
                }
            }
            return 1;
        case 'I':
            if (checkArgAndWarn(opt, arg, 0, 1, id()))
            {
                if (tryParseInt(arg, &intarg) && intarg >= 1)
                {
                    dos->dirInterleave = intarg;
                }
                else
                {
                    fprintf(stderr, "[cbmdos] Warning: invalid directory "
                            "interleave `%s' ignored.\n", arg);
                }
            }
            return 1;
        case 'D':
            if (checkArgAndWarn(opt, arg, 0, 1, id()))
            {
                if (tryParseIntHex(arg, &uintarg) && uintarg <= 0xff)
                {
                    _setDosVersion(dos, (uint8_t)uintarg);
                }
                else
                {
                    fprintf(stderr, "[cbmdos] Warning: invalid DOS version "
                            "`%s' ignored.\n", arg);
                }
            }
            return 1;
        case 'A':
            checkArgAndWarn(opt, arg, 0, 0, id());
            _allocateAll(dos);
            return 1;
        case '0':
            checkArgAndWarn(opt, arg, 0, 0, id());
            _setZeroFree(dos);
            return 1;
        default:
            return 0;
    }
}

static int
fileOption(IModule *self, DiskFile *file, char opt, const char *arg)
{
    Cbmdos *dos = (Cbmdos *)self;
    CbmdosFileData *data;
    int intarg;

    switch (opt)
    {
        case 'f':
            if (dos->reservedDirBlocks &&
                    !dos->directory && !dos->usedDirBlocks)
            {
                _reserveDirBlocks(dos);
            }
            DiskFile_setInterleave(file, 10);
            data = malloc(sizeof(CbmdosFileData));
            data->fileType = FT_PRG;
            data->writeDirEntry = 0;
            data->forceBlocks = -1;
            DiskFile_attachData(file, dos, data, &_deleteFileData);
            return 1;
        case 'n':
            data = DiskFile_data(file, dos);
            data->writeDirEntry = 1;
            if (arg)
            {
                DiskFile_setName(file, arg);
            }
            reserveDirSlot(dos);
            return 1;
        case 'T':
            if (!checkArgAndWarn(opt, arg, 1, 1, id())) return 1;
            data = DiskFile_data(file, dos);
            switch (arg[0])
            {
                case 'd':
                case 'D':
                    data->fileType = (data->fileType & FT_PROT) | FT_DEL;
                    break;
                case 's':
                case 'S':
                    data->fileType = (data->fileType & FT_PROT) | FT_SEQ;
                    break;
                case 'p':
                case 'P':
                    data->fileType = (data->fileType & FT_PROT) | FT_PRG;
                    break;
                case 'u':
                case 'U':
                    data->fileType = (data->fileType & FT_PROT) | FT_USR;
                    break;
                case 'r':
                case 'R':
                    data->fileType = (data->fileType & FT_PROT) | FT_REL;
                    break;
                default:
                    fprintf(stderr, "[cbmdos] Warning: unrecognized file type "
                            "`%s' ignored.\n", arg);
            }
            return 1;
        case 'P':
            checkArgAndWarn(opt, arg, 1, 0, id());
            data = DiskFile_data(file, dos);
            data->fileType |= FT_PROT;
            return 1;
        case 'S':
            checkArgAndWarn(opt, arg, 1, 1, id());
            if (tryParseInt(arg, &intarg) && intarg >= 0)
            {
                data = DiskFile_data(file, dos);
                data->forceBlocks = intarg;
            }
            else
            {
                fprintf(stderr, "[cbmdos] Warning: invalid value `%s' for "
                        "forced blocksize ignored.\n", arg);
            }
            return 1;
        default:
            return 0;
    }
}

static void
fileWritten(IModule *self, DiskFile *file, const BlockPosition *start)
{
    Cbmdos *dos = (Cbmdos *)self;
    CbmdosFileData *data;
    uint8_t *fileEntry;
    const char *fileName;
    size_t nameLen;
    int blockSize;
    static const char *unnamed = "----------------";

    DBGd2("cbmdos: fileWritten", start->track, start->sector);

    data = DiskFile_data(file, dos);

    if (!data->writeDirEntry) return;

    fileEntry = Block_rawData(dos->currentDirBlock)
        + dos->currentDirSlot * 0x20;
    memset(fileEntry, 0, 0x20);
    memset(fileEntry + 0x05, 0xa0, 0x10);
    if (dos->currentDirSlot == 0) fileEntry[1] = 0xff;

    fileEntry[0x02] = data->fileType;
    fileEntry[0x03] = start->track;
    fileEntry[0x04] = start->sector;

    fileName = DiskFile_name(file);
    if (!fileName) fileName = unnamed;
    nameLen = strlen(fileName);
    if (nameLen > 16) nameLen = 16;
    memcpy(fileEntry + 0x05, fileName, nameLen);

    if (data->forceBlocks >= 0)
    {
        blockSize = data->forceBlocks;
    }
    else
    {
        blockSize = DiskFile_blocks(file);
    }
    fileEntry[0x1e] = blockSize & 0xff;
    fileEntry[0x1f] = blockSize >> 8;

    dos->flags &= ~CDFL_SLOTRESERVED;
}

static void
statusChanged(IModule *self, const BlockPosition *pos)
{
    Cbmdos *dos = (Cbmdos *)self;
    uint8_t *bamEntry;
    int bamByte, bamBit;

    DBGd2("cbmdos: statusChanged", pos->track, pos->sector);

    if (dos->flags & CDFL_ALLOCALL) return;
    if (pos->track < 1 || pos->track > 35) return;

    bamEntry = Block_rawData(dos->bam) + 4 * pos->track;
    bamEntry[0] = dos->flags & CDFL_ZEROFREE ? 0 :
        Track_freeSectors(Image_track(dos->image, pos->track), ~BS_ALLOCATED);
    bamByte = pos->sector / 8 + 1;
    bamBit = pos->sector % 8;
    if (Image_blockStatus(dos->image, pos) & BS_ALLOCATED)
    {
        bamEntry[bamByte] &= ~(1<<bamBit);
    }
    else
    {
        bamEntry[bamByte] |= 1<<bamBit;
    }
}

static int
requestReservedBlock(IModule *self, const BlockPosition *pos)
{
    Cbmdos *dos = (Cbmdos *)self;
    DirBlock *parent = 0;
    DirBlock *current;

    /* never give first directory block away */
    if (pos->track == 18 && pos->sector == 1) return 0;

    for (current = dos->directory; current;
            parent = current, current = current->next)
    {
        if (current->pos.track == pos->track &&
                current->pos.sector == pos->sector)
        {
            if (parent) parent->next = current->next;
            else dos->directory = current->next;
            free(current);
            ++(dos->reclaimedDirBlocks);
            DBGd2("cbmdos: gave back block", pos->track, pos->sector);
            return 1;
        }
    }

    return 0;
}

static void
imageComplete(IModule *self)
{
    Cbmdos *dos = (Cbmdos *)self;
    char buf[8];

    if (dos->flags & CDFL_DIROVERFLOW) return;

    if (dos->extraDirBlocks)
    {
        snprintf(buf, 8, "%d", dos->reservedDirBlocks + dos->extraDirBlocks);
        Mkd64_suggestOption(MKD64, self, 0, 'R', buf,
                "More directory blocks were needed, reserving them beforehand "
                "lessens directory fragmentation.");
    }
    else if (dos->reclaimedDirBlocks &&
            dos->reservedDirBlocks - dos->reclaimedDirBlocks < 18)
    {
        snprintf(buf, 8, "%d",
                dos->reservedDirBlocks - dos->reclaimedDirBlocks);
        Mkd64_suggestOption(MKD64, self, 0, 'R', buf,
                "Blocks reserved for directory were needed for files. Not "
                "reserving them can lessen file fragmentation.");
    }
    else if (dos->reservedDirBlocks > 18 && dos->usedDirBlocks <=18)
    {
        buf[0] = '1';
        buf[1] = '8';
        buf[2] = 0;
        Mkd64_suggestOption(MKD64, self, 0, 'R', buf,
                "More than 18 directory blocks were reserved, but not needed. "
                "Not reserving them can lessen file fragmentation.");
    }
}

SOEXPORT IModule *
instance(void)
{
    if (singleInstance)
    {
        fputs("[cbmdos] ERROR: refusing to create a second instance.\n",
                stderr);
        return 0;
    }

    singleInstance = mkd64Alloc(sizeof(Cbmdos));
    memset(singleInstance, 0, sizeof(Cbmdos));

    singleInstance->mod.id = &id;
    singleInstance->mod.free = &delete;
    singleInstance->mod.initImage = &initImage;
    singleInstance->mod.globalOption = &globalOption;
    singleInstance->mod.fileOption = &fileOption;
    singleInstance->mod.fileWritten = &fileWritten;
    singleInstance->mod.statusChanged = &statusChanged;
    singleInstance->mod.requestReservedBlock = &requestReservedBlock;
    singleInstance->mod.imageComplete = &imageComplete;

    singleInstance->reservedDirBlocks = 18;
    singleInstance->dirInterleave = 3;
    singleInstance->currentDirSlot = 7; /* force new dir block allocation */

    singleInstance->alloc = cbmdosAllocator_new();
    
    return (IModule *) singleInstance;
}

SOEXPORT const char *
help(void)
{
    return
"cbmdos implements the default directory and BAM scheme of a 1541 floppy.\n"
"Interleave is initially set to 10 for every file (cbmdos standard). The\n"
"following options are recognized:\n\n"
"  -d DISKNAME   The name of the disk, defaults to an empty name.\n"
"  -i DISKID     The ID of the disk, defaults to two random characters.\n"
"                this can be up to 5 characters long, in this case it will\n"
"                overwrite the default `DOS type' string (`2A').\n"
"  -R DIRBLOCKS  reserve {DIRBLOCKS} blocks for the directory. The default\n"
"                value is 18, which is exactly the whole track #18.\n"
"  -I INTERLV    Set the directory interleave to {INTERLV}. The default value\n"
"                for directory interleave is 3.\n"
"  -D DOSVER     Set the dos version byte to {DOSVER}, given in hexadecimal.\n"
"                The default value is (hex) 41. this can be used for soft\n"
"                write protection, the original floppy will refuse any write\n"
"                attempts if this value is changed.\n"
"  -A            Allocate all blocks in the BAM.\n"
"  -0            Set available blocks to 0 in BAM, but still write flags for\n"
"                individual sectors.\n";
}

SOEXPORT const char *
helpFile(void)
{
    return
"  -n [FILENAME] Activates cbmdos directory entry for the current file. If\n"
"                {FILENAME} is given, it is used for the cbmdos directory.\n"
"  -T FILETYPE   One of `p', `s', `u', `r' or `d' (for PRG, SEQ, USR, REL or\n"
"                DEL), defaults to PRG.\n"
"  -P            Make the file write-protected.\n"
"  -S BLOCKSIZE  Force the size written to the directory to be {BLOCKSIZE}.\n";
}

SOEXPORT const char *
versionInfo(void)
{
    return
"cbmdos " MODVERSION "\n"
"an mkd64 module for writing original-like BAM and Directory.\n"
"Felix Palmen (Zirias) -- <felix@palmen-it.de>\n\n"
BUILDID_ALL "\n";
}
/* vim: et:si:ts=4:sts=4:sw=4
*/
