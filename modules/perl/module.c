#include <mkd64/common.h>
#include <mkd64/imodule.h>
#include <mkd64/mkd64.h>
#include <mkd64/modrepo.h>
#include <mkd64/util.h>
#include <mkd64/diskfile.h>
#include <mkd64/debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buildid.h"

#include <EXTERN.h>
#include <perl.h>

MKD64_MODULE("perl")

#define MODVERSION "1.3b"

#define my_perl mod->pi

static int instances = 0;

static int fakeargc = 1;
static char **fakeargv = { 0 };
static char **fakeenv = { 0 };

typedef struct
{
    IModule mod;
    PerlInterpreter *pi;
    Image *img;
} Perl;

static void
delete(IModule *self)
{
    Perl *mod = (Perl *)self;

    PERL_SET_CONTEXT(mod->pi);
    perl_destruct(mod->pi);
    perl_free(mod->pi);
    if (--instances == 0) PERL_SYS_TERM();

    DBG("[perl] interpreter destroyed.");

    free(mod);
}

static void
initImage(IModule *self, Image *image)
{
    Perl *mod = (Perl *)self;

    mod->img = image;
}
static int
option(IModule *self, char opt, const char *arg)
{
    Perl *mod = (Perl *)self;
    static char *perl_argv[] = { "", 0, 0 };

    PERL_SET_CONTEXT(mod->pi);
    if (opt == 'e')
    {
        PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
        perl_argv[1] = copyString(arg);
        perl_parse(mod->pi, 0, 2, perl_argv, 0);
        free(perl_argv[1]);
    }
}

static int
fileOption(IModule *self, DiskFile *file, char opt, const char *arg)
{
    Perl *mod = (Perl *)self;
    return 0;
}

SOEXPORT IModule *
instance(void)
{
    Perl *mod = mkd64Alloc(sizeof(Perl));
    memset(mod, 0, sizeof(Perl));

    if (++instances == 1) PERL_SYS_INIT3(&fakeargc, &fakeargv, &fakeenv);
    mod->pi = perl_alloc();
    PERL_SET_CONTEXT(mod->pi);
    perl_construct(mod->pi);

    DBG("[perl] interpreter created");

    mod->mod.id = &id;
    mod->mod.free = &delete;
    mod->mod.initImage = &initImage;
    mod->mod.fileOption = &fileOption;

    return (IModule *) mod;
}

SOEXPORT const char *
help(void)
{
    return "perl.\n";
}

SOEXPORT const char *
versionInfo(void)
{
    return
"perl " MODVERSION "\n"
"an mkd64 module for implementing modules in perl.\n"
"Felix Palmen (Zirias) -- <felix@palmen-it.de>\n\n"
BUILDID_ALL "\n";
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
