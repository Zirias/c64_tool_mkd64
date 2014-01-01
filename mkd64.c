
#include "mkd64.h"
#include "image.h"
#include "cmdline.h"
#include "modrepo.h"

#include <stdio.h>

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

static void
printVersion(void)
{
    fputs("mkd64 " MKD64_VERSION "\n"
            "a modular tool for creating D64 disk images.\n"
            "Felix Palmen (Zirias) -- <felix@palmen-it.de>\n", stderr);
}

static void
printUsage(void)
{
    const char *exe = cmdline_exe(mkd64.cmdline);
    printVersion();
    fprintf(stderr, "\nUSAGE: %s OPTION [ARGUMENT] [OPTION [ARGUMENT]...]\n"
            "       [FILEOPTION [ARGUMENT]...]\n\n"
            "type `%s -?' for help on available options and fileoptions.\n",
            exe, exe);
}

int
mkd64_run(void)
{
    if (!mkd64.initialized) return 0;

    if (!cmdline_moveNext(mkd64.cmdline))
    {
        printUsage();
        return 0;
    }

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

int main(int argc, char **argv)
{
    mkd64_init(argc, argv);
    mkd64_run();
    mkd64_done();
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
