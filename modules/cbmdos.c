
#include "../module.h"
#include "../imodule.h"

static const char *modid = "cbmdos";

typedef struct
{
    IModule mod;
} Cbmdos;

static void
initImage(IModule *this, Image *image)
{
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
fileWritten(IModule *this, Diskfile *file)
{
}

static void
statusChanged(IModule *this, BlockPosition *block)
{
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
