#include <mkd64/common.h>
#include <mkd64/imodule.h>
#include <mkd64/debug.h>
#include <mkd64/block.h>
#include <mkd64/track.h>

#include "buildid.h"

#define MODVERSION "0.1b"

static const char *modid = "dolphindos";

static const char *moddepends[] = { "cbmdos", 0 };

typedef struct
{
    IModule mod;
    Image *image;
    Block *bam;
    Track *extraTracks[5];
} Dolphindos;

static void
initImage(IModule *this, Image *image)
{
    Dolphindos *dos = (Dolphindos *)this;
    BlockPosition pos = { 18, 0 };
    uint8_t *bamEntry;
    int i;

    dos->image = image;
    dos->bam = image_block(image, &pos);
    for (i = 0; i < 5; ++i)
    {
        if (!image_track(image, i+36))
        {
            dos->extraTracks[i] = track_new(i+36, 17);
        }
        bamEntry = block_rawData(dos->bam) + 0xac + 4*i;
        bamEntry[0] = 0x11;
        bamEntry[1] = 0xff;
        bamEntry[2] = 0xff;
        bamEntry[3] = 0x01;
    }
}

static Track *
getTrack(IModule *this, int track)
{
    Dolphindos *dos = (Dolphindos *)this;
    if (track > 35 && track <= 40) return dos->extraTracks[track-36];
    return 0;
}

static void
statusChanged(IModule *this, const BlockPosition *pos)
{
    Dolphindos *dos = (Dolphindos *)this;
    uint8_t *bamEntry;
    int bamByte, bamBit;

    DBGd2("dolphindos: statusChanged", pos->track, pos->sector);

    if (pos->track < 35 || pos->track > 40) return;
    if (!dos->extraTracks[pos->track - 36]) return;

    bamEntry = block_rawData(dos->bam) + 0xac + 4 * (pos->track-36);
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
    Dolphindos *this = calloc(1, sizeof(Dolphindos));
    this->mod.id = &id;
    this->mod.initImage = &initImage;
    this->mod.getTrack = &getTrack;
    this->mod.statusChanged = &statusChanged;

    return (IModule *) this;
}

SOEXPORT void
delete(IModule *instance)
{
    Dolphindos *this = (Dolphindos *) instance;
    int i;

    for (i = 0; i < 5; ++i)
    {
        if (this->extraTracks[i]) track_delete(this->extraTracks[i]);
    }

    free(this);
}

SOEXPORT const char **
depends(void)
{
    return moddepends;
}

SOEXPORT const char *
help(void)
{
    return
"dolphindos provides tracks 36 - 40 in DOLPHIN DOS format. This is an\n"
"extension to cbmdos, so this module is required. Options are not provided,\n"
"see help for cbmdos for options.\n";
}

SOEXPORT const char *
versionInfo(void)
{    return
"dolphindos " MODVERSION "\n"
"an mkd64 module that extends cbmdos, providing extra tracks.\n"
"Felix Palmen (Zirias) -- <felix@palmen-it.de>\n\n"
BUILDID_ALL "\n";
}
/* vim: et:si:ts=4:sts=4:sw=4
*/
