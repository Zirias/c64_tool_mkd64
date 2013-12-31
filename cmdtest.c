
#include "mkd64.h"

#include <stdio.h>

const char *noarg = "<EMPTY>";

int main(int argc, char **argv)
{
    Cmdline *cl;

    mkd64_init(argc, argv);

    cl = mkd64_cmdline();
    while (cmdline_moveNext(cl))
    {
        char opt = cmdline_opt(cl);
        const char *arg = cmdline_arg(cl);
        printf("%c: %s\n", opt, arg?arg:noarg);
    }

    mkd64_done();
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
