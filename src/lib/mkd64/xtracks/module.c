#include <mkd64/common.h>
#include <mkd64/imodule.h>
#include <mkd64/debug.h>
#include <mkd64/block.h>
#include <mkd64/track.h>
#include <mkd64/util.h>

#include <stdio.h>
#include <string.h>

#include "buildid.h"

MKD64_MODULE("xtracks")

#define MODVERSION "1.4b"

typedef struct
{
    IModule mod;
    Image *image;
    Block *bam;
    Track *extraTracks[5];
    int doSpeedDosBam;
    int doDolphinDosBam;
} Xtracks;

/* allow only one instance */
static Xtracks *singleInstance = 0;

static void
delete(IModule *self)
{
    Xtracks *dos = (Xtracks *) self;
    int i;

    for (i = 0; i < 5; ++i)
    {
        if (dos->extraTracks[i]) OBJDEL(Track, dos->extraTracks[i]);
    }

    free(dos);
}

static void
initImage(IModule *self, Image *image)
{
    Xtracks *dos = (Xtracks *)self;
    int i;

    dos->image = image;
    for (i = 0; i < 5; ++i)
    {
        if (!Image_track(image, i+36))
        {
            if (dos->extraTracks[i]) OBJDEL(Track, dos->extraTracks[i]);
            dos->extraTracks[i] = OBJNEW2(Track, i+36, 17);
        }
    }
}

static void
getBam(Xtracks *self)
{
    BlockPosition pos = { 18, 0 };

    if (!self->bam)
    {
        self->bam = Image_block(self->image, &pos);
    }

    if (!(Block_status(self->bam) & BS_ALLOCATED))
    {
        Block_allocate(self->bam);
    }
}

static int
option(IModule *self, char opt, const char *arg)
{
    Xtracks *dos = (Xtracks *)self;
    const char *argopt;
    uint8_t *bamEntry;
    int i;

    if (opt == 'X')
    {
        if (checkArgAndWarn(opt, arg, 0, 1, id()))
        {
            for (argopt = arg; *argopt; ++argopt)
            {
                if (*argopt == 'd' || *argopt == 'D')
                {
                    getBam(dos);
                    for (i = 0; i < 5; ++i)
                    {
                        bamEntry = Block_rawData(dos->bam) + 0xac + 4*i;
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
                        bamEntry = Block_rawData(dos->bam) + 0xc0 + 4*i;
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
getTrack(IModule *self, int track)
{
    Xtracks *dos = (Xtracks *)self;
    if (track > 35 && track <= 40) return dos->extraTracks[track-36];
    return 0;
}

static void
updateBam(Xtracks *self, uint8_t *bamEntry, const BlockPosition *pos)
{
    int bamByte, bamBit;

    bamEntry[0] = Track_freeSectors(Image_track(self->image, pos->track),
            ~BS_ALLOCATED);
    bamByte = pos->sector / 8 + 1;
    bamBit = pos->sector % 8;
    if (Image_blockStatus(self->image, pos) & BS_ALLOCATED)
    {
        bamEntry[bamByte] &= ~(1<<bamBit);
    }
    else
    {
        bamEntry[bamByte] |= 1<<bamBit;
    }
}

static void
statusChanged(IModule *self, const BlockPosition *pos)
{
    Xtracks *dos = (Xtracks *)self;
    uint8_t *bamEntry;

    DBGd2("xtracks: statusChanged", pos->track, pos->sector);

    if (pos->track < 35 || pos->track > 40) return;
    if (!dos->extraTracks[pos->track - 36]) return;

    if (dos->doDolphinDosBam)
    {
        bamEntry = Block_rawData(dos->bam) + 0xac + 4 * (pos->track-36);
        updateBam(dos, bamEntry, pos);
    }

    if (dos->doSpeedDosBam)
    {
        bamEntry = Block_rawData(dos->bam) + 0xc0 + 4 * (pos->track-36);
        updateBam(dos, bamEntry, pos);
    }
}

SOEXPORT IModule *
instance(void)
{
    if (singleInstance)
    {
        fputs("[xtracks] ERROR: refusing to create a second instance.\n",
                stderr);
        return 0;
    }

    singleInstance = mkd64Alloc(sizeof(Xtracks));
    memset(singleInstance, 0, sizeof(Xtracks));

    singleInstance->mod.id = &id;
    singleInstance->mod.free = &delete;
    singleInstance->mod.initImage = &initImage;
    singleInstance->mod.option = &option;
    singleInstance->mod.getTrack = &getTrack;
    singleInstance->mod.statusChanged = &statusChanged;

    return (IModule *) singleInstance;
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
