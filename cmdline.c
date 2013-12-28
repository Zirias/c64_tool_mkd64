
#include "cmdline.h"

#include <stdlib.h>
#include <string.h>

struct cmdline
{
    int count;
    int pos;
    char *opts;
    char **args;
};

static void
clear(Cmdline *this)
{
    int i;
    if (this->args)
    {
        for (i = 0; i < this->count; ++i)
        {
            free(this->args[i]);
        }
        free(this->args);
        this->args = 0;
    }
    free(this->opts);
    this->opts = 0;
    this->count = 0;
    this->pos = 0;
}

Cmdline *
cmdline_new(void)
{
    Cmdline *this = calloc(1, sizeof(Cmdline));
    return this;
}

void
cmdline_delete(Cmdline *this)
{
    clear(this);
    free(this);
}

void
cmdline_parse(Cmdline *this, int argc, char **argv)
{
    char **argvp;

    clear(this);

    this->opts = malloc(argc * sizeof(char));
    this->args = malloc(argc * sizeof(char *));

    for (argvp = argv+1; *argvp; ++argvp)
    {
        if (strlen(*argvp) > 1 && **argvp == '-')
        {
            this->opts[this->count] = (*argvp)[1];
            if (strlen(*argvp) > 2)
            {
                this->args[this->count] = strdup((*argvp)+2);
            }
            else if (*(argvp+1) && **(argvp+1) != '-')
            {
                this->args[this->count] = strdup(*++argvp);
            }
            else
            {
                this->args[this->count] = 0;
            }
            ++(this->count);
        }
    }
}

char
cmdline_opt(const Cmdline *this)
{
    return this->opts[this->pos];
}

const char *
cmdline_arg(const Cmdline *this)
{
    return this->args[this->pos];
}

int
cmdline_moveNext(Cmdline *this)
{
    ++(this->pos);
    if (this->pos == this->count)
    {
        this->pos = 0;
        return 0;
    }
    return 1;
}

/* vim: et:si:ts=8:sts=4:sw=4
*/
