#include <mkd64/common.h>

#include <mkd64/util.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

SOEXPORT int
randomNum(int min, int max)
{
    static int initialized = 0;

    if (!initialized)
    {
        srand(time(NULL));
        initialized = 1;
    }

    return (int) (rand() / ((double)RAND_MAX+1) * (max-min+1) + min);
}

SOEXPORT int
tryParseInt(const char *str, int *result)
{
    int negative = 0;
    const char *p = str;

    if (!p) return 0;

    *result = 0;
    if (*p == '-')
    {
        negative = 1;
        ++p;
    }

    while (*p)
    {
        if (*p >= '0' && *p <= '9')
        {
            *result *= 10;
            *result += (*p - '0');
        }
        else
        {
            return 0;
        }
        ++p;
    }

    if (negative) *result = -*result;
    return 1;
}

SOEXPORT int
checkArgAndWarn(char opt, const char *arg, int isFileOpt,
        int argExpected, const char *modid)
{
    static const char *globalOpt = "global";
    static const char *fileOpt = "file";

    static const char *formatMissing = "Warning: missing argument for %s "
        "option -%c, option ignored.\n";
    static const char *formatMissingMod = "[%s] Warning: missing argument "
        "for %s option -%c, option ignored.\n";
    static const char *formatExtra = "Warning: ignored unexpected argument "
        "`%s' to %s option -%c\n";
    static const char *formatExtraMod = "[%s] Warning: ignored unexpected "
        "argument `%s' to %s option -%c\n";

    const char *optType = isFileOpt ? fileOpt : globalOpt;

    if (argExpected && !arg)
    {
        if (modid) fprintf(stderr, formatMissingMod, modid, optType, opt);
        else fprintf(stderr, formatMissing, optType, opt);
        return 0;
    }
    if (!argExpected && arg)
    {
        if (modid) fprintf(stderr, formatExtraMod, modid, arg, optType, opt);
        else fprintf(stderr, formatExtra, arg, optType, opt);
        return 0;
    }
    return 1;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/