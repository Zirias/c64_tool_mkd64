
#include "mkd64.h"
#include "image.h"
#include "cmdline.h"
#include "modrepo.h"

typedef struct
{
    int initialized;
    Image *image;
    Cmdline *cmdline;
    Modrepo *modrepo;
} Mkd64;

static Mkd64 mkd64 = {0};

int
mkd64_init(int argc, char **argv)
{
    mkd64.image = image_new();
    mkd64.cmdline = cmdline_new();
    cmdline_parse(mkd64.cmdline, argc, argv);
    mkd64.modrepo = modrepo_new(cmdline_exe(mkd64.cmdline));
    mkd64.initialized = 1;
    return 1;
}

int
mkd64_run(void)
{
    if (!mkd64.initialized) return 0;
    return 1;
}

void
mkd64_done(void)
{
    if (!mkd64.initialized) return;
    mkd64.initialized = 0;
    modrepo_delete(mkd64.modrepo);
    cmdline_delete(mkd64.cmdline);
    image_delete(mkd64.image);
}

Image *
mkd64_image(void)
{
    return mkd64.initialized ? mkd64.image : 0;
}

Cmdline *
mkd64_cmdline(void)
{
    return mkd64.initialized ? mkd64.cmdline : 0;
}

Modrepo *
mkd64_modrepo(void)
{
    return mkd64.initialized ? mkd64.modrepo : 0;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
