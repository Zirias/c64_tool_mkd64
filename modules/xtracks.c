#include <mkd64/common.h>
#include <mkd64/imodule.h>
#include <mkd64/debug.h>
#include <mkd64/block.h>
#include <mkd64/track.h>
#include <mkd64/util.h>

#include <stdio.h>

#include "buildid.h"

#define MODVERSION "1.0"

static const char *modid = "xtracks";

typedef struct
{
    IModule mod;
    Image *image;
    Block *bam;
    Track *extraTracks[5];
    int doSpeedDosBam;
    int doDolphinDosBam;
} Xtracks;

static void
delete(IModule *this)
{
    Xtracks *dos = (Xtracks *) this;
    int i;

    for (i = 0; i < 5; ++i)
    {
        if (dos->extraTracks[i]) track_delete(dos->extraTracks[i]);
    }

    free(dos);
}

static void
initImage(IModule *this, Image *image)
{
    Xtracks *dos = (Xtracks *)this;
    int i;

    dos->image = image;
    for (i = 0; i < 5; ++i)
    {
        if (!image_track(image, i+36))
        {
            dos->extraTracks[i] = track_new(i+36, 17);
        }
    }
}

static void
getBam(Xtracks *this)
{
    BlockPosition pos = { 18, 0 };

    if (!this->bam)
    {
        this->bam = image_block(this->image, &pos);
    }

    if (!block_status(this->bam) & BS_ALLOCATED)
    {
        block_allocate(this->bam);
    }
}

static int
globalOption(IModule *this, char opt, const char *arg)
{
    Xtracks *dos = (Xtracks *)this;
    const char *argopt;
    uint8_t *bamEntry;
    int i;

    if (opt == 'X')
    {
        if (checkArgAndWarn(opt, arg, 0, 1, modid))
        {
            for (argopt = arg; *argopt; ++argopt)
            {
                if (*argopt == 'd' || *argopt == 'D')
                {
                    getBam(dos);
                    for (i = 0; i < 5; ++i)
                    {
                        bamEntry = block_rawData(dos->bam) + 0xac + 4*i;
                        bamEntry[0] = 0x11;
                        bamEntry[1] = 0xff;
                        bamEntry[2] = 0xff;
                        bamEntry[3] = 0x01;
                    }
                    dos->doDolphinDosBam = 1;
                }
                else if (*argopt == 's' || *argopt == 'S')
                {
                    getBam(dos);
                    for (i = 0; i < 5; ++i)
                    {
                        bamEntry = block_rawData(dos->bam) + 0xc0 + 4*i;
                        bamEntry[0] = 0x11;
                        bamEntry[1] = 0xff;
                        bamEntry[2] = 0xff;
                        bamEntry[3] = 0x01;
                    }
                    dos->doSpeedDosBam = 1;
                }
                else
                {
                    fprintf(stderr, "[xtracks] Warning: unknown extended bam "
                            "entry type `%c' ignored.\n", *argopt);
                }
            }
        }
        return 1;
    }

    return 0;
}

static Track *
getTrack(IModule *this, int track)
{
    Xtracks *dos = (Xtracks *)this;
    if (track > 35 && track <= 40) return dos->extraTracks[track-36];
    return 0;
}

static void
updateBam(Xtracks *this, uint8_t *bamEntry, const BlockPosition *pos)
{
    int bamByte, bamBit;

    bamEntry[0] = track_freeSectorsRaw(image_track(this->image, pos->track));
    bamByte = pos->sector / 8 + 1;
    bamBit = pos->sector % 8;
    if (image_blockStatus(this->image, pos) & BS_ALLOCATED)
    {
        bamEntry[bamByte] &= ~(1<<bamBit);
    }
    else
    {
        bamEntry[bamByte] |= 1<<bamBit;
    }
}

static void
statusChanged(IModule *this, const BlockPosition *pos)
{
    Xtracks *dos = (Xtracks *)this;
    uint8_t *bamEntry;

    DBGd2("xtracks: statusChanged", pos->track, pos->sector);

    if (pos->track < 35 || pos->track > 40) return;
    if (!dos->extraTracks[pos->track - 36]) return;

    if (dos->doDolphinDosBam)
    {
        bamEntry = block_rawData(dos->bam) + 0xac + 4 * (pos->track-36);
        updateBam(dos, bamEntry, pos);
    }

    if (dos->doSpeedDosBam)
    {
        bamEntry = block_rawData(dos->bam) + 0xc0 + 4 * (pos->track-36);
        updateBam(dos, bamEntry, pos);
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
    Xtracks *this = calloc(1, sizeof(Xtracks));
    this->mod.id = &id;
    this->mod.delete = &delete;
    this->mod.initImage = &initImage;
    this->mod.globalOption = &globalOption;
    this->mod.getTrack = &getTrack;
    this->mod.statusChanged = &statusChanged;

    return (IModule *) this;
}

SOEXPORT const char *
help(void)
{
    return
"xtracks provides tracks 36 - 40. Optionally, BAM entries can be written in\n"
"either DOLPHIN DOS or SPEED DOS format, or even both. The following option\n"
"controls xtracks behaviour:\n\n"
" -X BAMTYPE  One of `d' for DOLPHIN DOS or `s' for SPEED DOS, or both. If\n"
"             given, extended BAM entries are written for the extra tracks.\n"
"             ATTENTION: If you want xtracks to write BAM entries, make sure\n"
"             cbmdos is loaded before you give this option. Otherwise, cbmdos\n"
"             overwrites the complete BAM again.\n";
}

SOEXPORT const char *
versionInfo(void)
{    return
"xtracks " MODVERSION "\n"
"an mkd64 module providing tracks 36 - 40.\n"
"Felix Palmen (Zirias) -- <felix@palmen-it.de>\n\n"
BUILDID_ALL "\n";
}
/* vim: et:si:ts=4:sts=4:sw=4
*/
