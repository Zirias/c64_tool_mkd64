
#include "../module.h"
#include "../imodule.h"
#include "../debug.h"
#include "../block.h"
#include "../track.h"

static const char *modid = "cbmdos";

typedef struct
{
    IModule mod;
    Image *image;
    Block *bam;
    Block *directory[18];
    int currentDirSector;
} Cbmdos;

static void
initImage(IModule *this, Image *image)
{
    Cbmdos *dos = (Cbmdos *)this;
    BlockPosition pos = { 18, 0 };
    int i;
    uint8_t *data;

    dos->image = image;
    dos->bam = image_block(image, &pos);
    dos->currentDirSector = 0;
    block_reserve(dos->bam);

    for (i = 0; i < 18; ++i)
    {
        ++(pos.sector);
        dos->directory[i] = image_block(image, &pos);
        block_reserve(dos->directory[i]);
    }

    data = block_rawData(dos->bam);

}

static void
globalOption(IModule *this, char opt, const char *arg)
{
}

static void
fileOption(IModule *this, Diskfile *file, char opt, const char *arg)
{
}

static void
fileWritten(IModule *this, Diskfile *file, const BlockPosition *start)
{
    DBGd2("cbmdos: fileWritten", start->track, start->sector);
}

static void
statusChanged(IModule *this, const BlockPosition *pos)
{
    DBGd2("cbmdos: statusChanged", pos->track, pos->sector);
}

const char *
id(void)
{
    return modid;
}

IModule *
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

void
delete(IModule *instance)
{
    free(instance);
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
