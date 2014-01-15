#include <mkd64/common.h>
#include <mkd64/util.h>

#include "cmdline.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

#define FILEOPT_CHUNKSIZE 256

struct Cmdline
{
    int count;
    int pos;
    char *opts;
    char **args;
    char *exe;
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
    this->pos = -1;
}

SOLOCAL size_t
Cmdline_objectSize(void)
{
    return sizeof(Cmdline);
}

SOLOCAL Cmdline *
Cmdline_init(Cmdline *this)
{
    this->count = 0;
    this->pos = -1;
    this->opts = 0;
    return this;
}

SOLOCAL void
Cmdline_done(Cmdline *this)
{
    clear(this);
}

static void
_warnLooseArg(const char *arg)
{
    fprintf(stderr, "Warning: loose argument `%s' ignored.\n", arg);
}

SOLOCAL void
Cmdline_parse(Cmdline *this, int argc, char **argv)
{
    char **argvp;

    clear(this);

    this->exe = *argv;
    this->opts = malloc(argc * sizeof(char));
    this->args = malloc(argc * sizeof(char *));

    for (argvp = argv+1; *argvp; ++argvp)
    {
        if (strlen(*argvp) > 1 && **argvp == '-')
        {
            this->opts[this->count] = (*argvp)[1];
            if (strlen(*argvp) > 2)
            {
                this->args[this->count] = copyString((*argvp)+2);
            }
            else if (*(argvp+1) && **(argvp+1) != '-')
            {
                this->args[this->count] = copyString(*++argvp);
            }
            else
            {
                this->args[this->count] = 0;
            }
            ++(this->count);
        }
        else _warnLooseArg(*argvp);
    }
}

static int
_charIn(const char *set, char c)
{
    const char *ptr = set;
    while (*ptr)
    {
        if (*ptr == c) return 1;
        ++ptr;
    }
    return 0;
}

static void
_rmFirstChar(char *s)
{
    char *ptr = s;
    while (ptr[1])
    {
        *ptr = ptr[1];
        ++ptr;
    }
}

static char *
_cmdtok(char *str, const char *delim, const char *quote)
{
    static char *start;
    char *ptr, *tok;
    char inquot;

    if (str) start = str;
    if (*start == '\0') return 0;

    inquot = '\0';

    while (_charIn(delim, *start)) ++start;
    if (*start == '\0') return 0;

    ptr = start;
    while (*ptr != '\0' && !_charIn(delim, *ptr))
    {
        if (_charIn(quote, *ptr))
        {
            inquot = *ptr;
            _rmFirstChar(ptr);
            while (*ptr != '\0' && *ptr != inquot)
            {
                ++ptr;
            }
            if (*ptr == inquot) _rmFirstChar(ptr);
            inquot = '\0';
        }
        else
        {
            ++ptr;
        }
    }
    tok = start;
    if (*ptr == '\0')
    {
        start = ptr;
    }
    else
    {
        *ptr = '\0';
        start = ptr+1;
    }
    return tok;
}

SOLOCAL int
Cmdline_parseFile(Cmdline *this, const char *cmdfile)
{
    static const char *delim = " \t\r\n";
    static const char *quote = "\"'";
    static struct stat st;
    size_t optSize = FILEOPT_CHUNKSIZE * sizeof(char);
    size_t argSize = FILEOPT_CHUNKSIZE * sizeof(char *);
    char *buf, *tok;
    FILE *f;

    clear(this);

    if (stat(cmdfile, &st) < 0) return 0;
    if (st.st_size < 1) return 0;
    if (!(f = fopen(cmdfile, "rb"))) return 0;

    buf = malloc(st.st_size);

    if (fread(buf, 1, st.st_size, f) != st.st_size)
    {
        fclose(f);
        free(buf);
        return 0;
    }

    this->opts = malloc(optSize);
    this->args = malloc(argSize);

    tok = _cmdtok(buf, delim, quote);
    while (tok)
    {
        if (strlen(tok) > 1 && *tok == '-')
        {
            this->opts[this->count] = tok[1];
            if (strlen(tok) > 2)
            {
                this->args[this->count] = copyString(tok+2);
                tok = _cmdtok(0, delim, quote);
            }
            else
            {
                tok = _cmdtok(0, delim, quote);
                if (tok && tok[0] != '-')
                {
                    this->args[this->count] = copyString(tok);
                    tok = _cmdtok(0, delim, quote);
                }
                else
                {
                    this->args[this->count] = 0;
                }
            }
            if (!(++(this->count) % FILEOPT_CHUNKSIZE))
            {
                optSize += FILEOPT_CHUNKSIZE * sizeof(char);
                this->opts = realloc(this->opts, optSize);
                argSize += FILEOPT_CHUNKSIZE * sizeof(char *);
                this->args = realloc(this->args, argSize);
            }
        }
        else
        {
            _warnLooseArg(tok);
            tok = _cmdtok(0, delim, quote);
        }
    }
    fclose(f);
    free(buf);
    return 1;
}

SOLOCAL char
Cmdline_opt(const Cmdline *this)
{
    if (this->pos < 0) return '\0';
    return this->opts[this->pos];
}

SOLOCAL const char *
Cmdline_arg(const Cmdline *this)
{
    if (this->pos < 0) return 0;
    return this->args[this->pos];
}

SOLOCAL int
Cmdline_moveNext(Cmdline *this)
{
    ++(this->pos);
    if (this->pos == this->count)
    {
        this->pos = -1;
        return 0;
    }
    return 1;
}

SOLOCAL const char *
Cmdline_exe(const Cmdline *this)
{
    return this->exe;
}

SOLOCAL int
Cmdline_count(const Cmdline *this)
{
    return this->count;
}
/* vim: et:si:ts=4:sts=4:sw=4
*/
