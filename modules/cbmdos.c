#include <mkd64/common.h>
#include <mkd64/imodule.h>
#include <mkd64/debug.h>
#include <mkd64/block.h>
#include <mkd64/track.h>
#include <mkd64/random.h>

#include <stdio.h>
#include <string.h>

#include "buildid.h"

#define MODVERSION "0.2b"

static const char *modid = "cbmdos";

typedef struct
{
    IModule mod;
    Image *image;
    Block *bam;
    Block *directory[18];
    int currentDirSector;
    int currentDirSlot;
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
} CbmdosFileData;

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

static const int _dirSectors[18] = {
    1,5,9,13,17,2,6,10,14,18,3,7,11,15,4,8,12,16
};

static void
initImage(IModule *this, Image *image)
{
    Cbmdos *dos = (Cbmdos *)this;
    BlockPosition pos = { 18, 0 };
    int i;
    uint8_t *data;

    dos->image = image;
    dos->bam = image_block(image, &pos);
    dos->currentDirSector = -1;
    dos->currentDirSlot = 7;

    for (i = 0; i < 18; ++i)
    {
        ++(pos.sector);
        dos->directory[i] = image_block(image, &pos);
        block_reserve(dos->directory[i]);
    }

    data = block_rawData(dos->bam);
    memcpy(data, _initialBam, 256);

    data[0xa2] = random_num(0x30, 0x53);
    if (data[0xa2] > 0x39) data[0xa2] += 7;
    data[0xa3] = random_num(0x30, 0x53);
    if (data[0xa3] > 0x39) data[0xa3] += 7;

    block_allocate(dos->bam);
}

static void
globalOption(IModule *this, char opt, const char *arg)
{
    Cbmdos *dos = (Cbmdos *)this;
    int arglen;

    switch (opt)
    {
        case 'd':
            if (arg)
            {
                arglen = strlen(arg);
                if (arglen > 16) arglen = 16;
                memcpy(block_rawData(dos->bam) + 0x90, arg, arglen);
            }
            break;
        case 'i':
            if (arg)
            {
                arglen = strlen(arg);
                if (arglen > 5) arglen = 5;
                memcpy(block_rawData(dos->bam) + 0xa2, arg, arglen);
            }
            break;
    }
}

static void
fileOption(IModule *this, Diskfile *file, char opt, const char *arg)
{
    Cbmdos *dos = (Cbmdos *)this;
    CbmdosFileData *data;

    switch (opt)
    {
        case 'f':
            diskfile_setInterleave(file, 10);
            data = malloc(sizeof(CbmdosFileData));
            data->fileType = FT_PRG;
            data->writeDirEntry = 0;
            diskfile_attachData(file, dos, data);
            break;
        case 'n':
            data = diskfile_data(file, dos);
            data->writeDirEntry = 1;
            if (arg)
            {
                diskfile_setName(file, arg);
            }
            break;
        case 't':
            if (!arg) break;
            data = diskfile_data(file, dos);
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
            }
            break;
        case 'P':
            data = diskfile_data(file, dos);
            data->fileType |= FT_PROT;
            break;
    }
}

static void
fileWritten(IModule *this, Diskfile *file, const BlockPosition *start)
{
    Cbmdos *dos = (Cbmdos *)this;
    CbmdosFileData *data;
    uint8_t *fileEntry;
    const char *fileName;
    size_t nameLen;
    BlockPosition dirBlockPos;
    Block *dirBlock;

    DBGd2("cbmdos: fileWritten", start->track, start->sector);

    data = diskfile_removeData(file, dos);

    if (!data->writeDirEntry)
    {
        free(data);
        return;
    }

    dirBlockPos.track = 18;
    ++(dos->currentDirSlot);
    if (dos->currentDirSlot > 7)
    {
        if (dos->currentDirSector > 17)
        {
            fputs("[cbmdos] ERROR: directory full!\n", stderr);
            --(dos->currentDirSlot);
            return;
        }
        dos->currentDirSlot = 0;
        ++(dos->currentDirSector);
        if (dos->currentDirSector > 0)
        {
            dirBlockPos.sector = _dirSectors[dos->currentDirSector - 1];
            fileEntry = block_rawData(image_block(dos->image, &dirBlockPos));
            fileEntry[0] = 18;
            fileEntry[1] = _dirSectors[dos->currentDirSector];
        }
        dirBlockPos.sector = _dirSectors[dos->currentDirSector];
        dirBlock = image_block(dos->image, &dirBlockPos);
        block_allocate(dirBlock);
        memset(block_rawData(dirBlock), 0, 256);
    }
    else
    {
        dirBlockPos.sector = _dirSectors[dos->currentDirSector];
        dirBlock = image_block(dos->image, &dirBlockPos);
    }

    fileEntry = block_rawData(dirBlock) + dos->currentDirSlot * 0x20;
    memset(fileEntry, 0, 0x20);
    memset(fileEntry + 0x05, 0xa0, 0x10);
    if (dos->currentDirSlot == 0) fileEntry[1] = 0xff;

    fileName = diskfile_name(file);

    fileEntry[0x02] = data->fileType;
    fileEntry[0x03] = start->track;
    fileEntry[0x04] = start->sector;

    nameLen = strlen(fileName);
    if (nameLen > 16) nameLen = 16;
    memcpy(fileEntry + 0x05, fileName, nameLen);

    fileEntry[0x1e] = diskfile_blocks(file) & 0xff;
    fileEntry[0x1f] = diskfile_blocks(file) >> 8;

    free(data);
}

static void
statusChanged(IModule *this, const BlockPosition *pos)
{
    Cbmdos *dos = (Cbmdos *)this;
    uint8_t *bamEntry;
    int bamByte, bamBit;

    DBGd2("cbmdos: statusChanged", pos->track, pos->sector);

    bamEntry = block_rawData(dos->bam) + 4 * pos->track;
    bamEntry[0] = track_freeSectorsRaw(image_track(dos->image, pos->track));
    bamByte = pos->sector / 8 + 1;
    bamBit = pos->sector % 8;
    if (image_blockStatus(dos->image, pos) & BS_ALLOCATED)
    {
        bamEntry[bamByte] &= ~(1<<bamBit);
    }
    else
    {
        bamEntry[bamByte] |= 1<<bamBit;
    }
}

SOEXPORT const char *
id(void)
{
    return modid;
}

SOEXPORT IModule *
instance(void)
{
    Cbmdos *this = malloc(sizeof(Cbmdos));
    this->mod.id = &id;
    this->mod.initImage = &initImage;
    this->mod.globalOption = &globalOption;
    this->mod.fileOption = &fileOption;
    this->mod.getTrack = 0;
    this->mod.fileWritten = &fileWritten;
    this->mod.statusChanged = &statusChanged;

    return (IModule *) this;
}

SOEXPORT void
delete(IModule *instance)
{
    free(instance);
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
"                This can be up to 5 characters long, in this case it will\n"
"                overwrite the default `DOS type' string (`2A')\n";
}

SOEXPORT const char *
helpFile(void)
{
    return
"  -n [FILENAME] Activates cbmdos directory entry for the current file. If\n"
"                {FILENAME} is given, it is used for the cbmdos directory.\n"
"  -t FILETYPE   One of `p', `s', `u', `r' or `d' (for PRG, SEQ, USR, REL or\n"
"                DEL), defaults to PRG.\n"
"  -P            Make the file write-protected.\n";
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
