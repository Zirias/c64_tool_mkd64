
#include "mkd64.h"
#include "modrepo.h"

#include <stdio.h>

const char *noarg = "<EMPTY>";

int main(int argc, char **argv)
{
    Cmdline *cl;
    Image *i;
    FILE *f;
    BlockPosition pos = {0,0};

    mkd64_init(argc, argv);

    modrepo_createInstance(mkd64_modrepo(), "cbmdos");

    cl = mkd64_cmdline();
    while (cmdline_moveNext(cl))
    {
        char opt = cmdline_opt(cl);
        const char *arg = cmdline_arg(cl);
        printf("%c: %s\n", opt, arg?arg:noarg);
    }

    Diskfile *testFile = diskfile_new();
    diskfile_setName(testFile, "testFile");
    diskfile_setInterleave(testFile, 10);
    
    f = fopen("modrepo.c", "r");
    if (f)
    {
        i = mkd64_image();
        diskfile_readFromHost(testFile, f);
        diskfile_write(testFile, i, &pos);
        filemap_dump(image_filemap(i), stderr);
    }

    mkd64_done();
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
