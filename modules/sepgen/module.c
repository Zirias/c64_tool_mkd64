#include <mkd64/common.h>
#include <mkd64/imodule.h>
#include <mkd64/mkd64.h>
#include <mkd64/modrepo.h>
#include <mkd64/util.h>
#include <mkd64/diskfile.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buildid.h"

MKD64_MODULE("separators")

#define MODVERSION "1.3b"

static const char *moddeps[] = { "cbmdos", 0 };

typedef struct
{
    const char *name;
    const unsigned char pattern[17];
    const int contentOffset;
    const size_t contentLen;
} SeparatorEntry;

typedef struct
{
    IModule mod;
    IModule *cbmdos;
} Separators;

/* allow only one instance */
static Separators *singleInstance = 0;

static const SeparatorEntry _seps[] = {
    { "simple",
        { 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d,
            0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0 },
        0,
        0
    },
    { "line",
        { 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
            0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0 },
        0,
        0
    },
    { "wave",
        { 0x66, 0x72, 0xaf, 0x72, 0x66, 0xc0, 0x64, 0x65,
            0x65, 0x64, 0x60, 0x66, 0x72, 0xaf, 0x72, 0x66, 0 },
        0,
        0
    },
    { "frtop",
        { 0xb0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
            0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xae, 0 },
        0,
        0
    },
    { "fr",
        { 0xdd, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
            0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xdd, 0 },
        1,
        14
    },
    { "frmid",
        { 0xab, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
            0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xb3, 0 },
        0,
        0
    },
    { "frbot",
        { 0xed, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
            0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xfd, 0 },
        0,
        0
    },
    { "roundtop",
        { 0x75, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
            0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x69, 0 },
        0,
        0
    },
    { "roundbot",
        { 0x6a, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
            0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x6b, 0 },
        0,
        0
    },
    { 0, {0}, 0, 0}
};

static void
delete(IModule *self)
{
    Separators *mod = (Separators *)self;
    free(mod);
}

SOEXPORT const char **
depends(void)
{
    return moddeps;
}

static int
fileOption(IModule *self, DiskFile *file, char opt, const char *arg)
{
    Separators *mod = (Separators *)self;
    const SeparatorEntry *sep;
    static char buf[17];
    const char *name;
    size_t nameLen;
    
    if (opt == 'p')
    {
        if (!checkArgAndWarn(opt, arg, 1, 1, id())) return 1;
        for (sep = _seps; sep->name; ++sep)
        {
            if (!strcmp(sep->name, arg)) break;
        }
        if (!sep->name)
        {
            fprintf(stderr, "[separators] Warning: unknown separator `%s' "
                    "ignored.\n", arg);
            return 1;
        }
        memcpy(buf, sep->pattern, 17);
        if (sep->contentLen)
        {
            name = DiskFile_name(file);
            nameLen = strlen(name);
            if (nameLen > sep->contentLen) nameLen = sep->contentLen;
            memcpy(buf + sep->contentOffset, name, nameLen);
        }
        mod->cbmdos->fileOption(mod->cbmdos, file, 'n', buf);
        return 1;
    }
    return 0;
}

SOEXPORT IModule *
instance(void)
{
    if (singleInstance)
    {
        fputs("[separators] ERROR: refusing to create a second instance.\n",
                stderr);
        return 0;
    }

    singleInstance = mkd64Alloc(sizeof(Separators));
    memset(singleInstance, 0, sizeof(Separators));

    singleInstance->mod.id = &id;
    singleInstance->mod.free = &delete;
    singleInstance->mod.fileOption = &fileOption;

    singleInstance->cbmdos = ModRepo_firstInstance(
            Mkd64_modRepo(MKD64), "cbmdos");

    return (IModule *) singleInstance;
}

SOEXPORT const char *
help(void)
{
    return
"separators allows easy creation of separators and boxes in cbmdos directory\n";
}

SOEXPORT const char *
helpFile(void)
{
    return
"  -p [NAME]  Set the name of the current file to a separator or box element\n"
"             defined by {NAME}. The following elements are available:\n"
"             * `simple': a separator consisting of `minus' (-) characters\n"
"             * `line': a solid line\n"
"             * `wave': a line looking like a wave\n"
"             * `frtop': the top border of a box with sharp edges\n"
"             * `fr': an element inside a box, the previously set filename\n"
"                     is included inside\n"
"             * `frmid': a solid line inside a box\n"
"             * `frbot': the bottom border of a box with sharp edges\n"
"             * `roundtop': the top border of a box with rounded edges\n"
"             * `roundbot': the bottom border of a box with rounded edges\n";
}

SOEXPORT const char *
versionInfo(void)
{
    return
"separators " MODVERSION "\n"
"an mkd64 module for easy creation of separators in cbmdos directory.\n"
"Felix Palmen (Zirias) -- <felix@palmen-it.de>\n\n"
BUILDID_ALL "\n";
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
