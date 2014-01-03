#include <mkd64/common.h>

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

struct cmdline
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

SOLOCAL Cmdline *
cmdline_new(void)
{
    Cmdline *this = calloc(1, sizeof(Cmdline));
    this->pos = -1;
    return this;
}

SOLOCAL void
cmdline_delete(Cmdline *this)
{
    clear(this);
    free(this);
}

SOLOCAL void
cmdline_parse(Cmdline *this, int argc, char **argv)
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

SOLOCAL void
cmdline_parseFile(Cmdline *this, FILE *cmdfile)
{
    static const char *delim = " \t\r\n";
    static const char *quote = "\"'";
    static struct stat st;
    size_t optSize = FILEOPT_CHUNKSIZE * sizeof(char);
    size_t argSize = FILEOPT_CHUNKSIZE * sizeof(char *);
    char *buf, *tok;

    clear(this);

    if (fstat(fileno(cmdfile), &st) < 0) return;
    if (st.st_size < 1) return;

    buf = malloc(st.st_size);
    rewind(cmdfile);

    if (fread(buf, 1, st.st_size, cmdfile) != st.st_size)
    {
        free(buf);
        return;
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
                this->args[this->count] = strdup(tok+2);
                tok = _cmdtok(0, delim, quote);
            }
            else
            {
                tok = _cmdtok(0, delim, quote);
                if (tok && tok[0] != '-')
                {
                    this->args[this->count] = strdup(tok);
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
            tok = _cmdtok(0, delim, quote);
        }
    }
    free(buf);
}

SOEXPORT char
cmdline_opt(const Cmdline *this)
{
    if (this->pos < 0) return '\0';
    return this->opts[this->pos];
}

SOEXPORT const char *
cmdline_arg(const Cmdline *this)
{
    if (this->pos < 0) return 0;
    return this->args[this->pos];
}

SOLOCAL int
cmdline_moveNext(Cmdline *this)
{
    ++(this->pos);
    if (this->pos == this->count)
    {
        this->pos = -1;
        return 0;
    }
    return 1;
}

SOEXPORT const char *
cmdline_exe(const Cmdline *this)
{
    return this->exe;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
