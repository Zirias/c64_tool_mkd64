#include <mkd64/common.h>

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

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
tryParseIntHex(const char *str, unsigned int *result)
{
    const char *p = str;

    if (!p) return 0;

    *result = 0;

    while (*p)
    {
        *result <<= 4;
        if (*p >= '0' && *p <= '9')
        {
            *result += (*p - '0');
        }
        else if (*p >= 'a' && *p <= 'f')
        {
            *result += (*p - 'a' + 0xa);
        }
        else if (*p >= 'A' && *p <= 'F')
        {
            *result += (*p - 'A' + 0xa);
        }
        else
        {
            return 0;
        }
        ++p;
    }

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

SOEXPORT char *
copyString(const char *s)
{
    size_t bytes = strlen(s)+1;
    char *copy = mkd64Alloc(bytes);
    memcpy(copy, s, bytes);
    return copy;
}

/* Helper function for conversion from hex character to number */
/* Returns -1 for illegal character input */
static int
hex2int(char hex) 
{
	if((hex < '0' || hex > '9') && (hex < 'a' || hex > 'f')) {
		return -1;
	}
	if(hex <= '9') {
		hex -= '0';
	} else {
		hex -= 'a'-10;
	}
	return (int)hex;
}

SOEXPORT int
parseHexEscapes(char *s)
{
	int read = 0, write = 0;

	while (s[read] != '\0') {
		if(s[read] == '$') {
			int hi = hex2int(s[++read]);
			int lo = hex2int(s[++read]);
			if(hi == -1 || lo == -1) {
				return 0;
			}
			s[write] = (unsigned char)(16 * hi + lo); 
		} else {
			s[write] = s[read];
		}
		read++;
		write++;
	}
	s[write] = s[read]; /* copy final \0 */
	return 1;
}

SOEXPORT int
stringEndsWith(const char *s, const char *expectedEnd, int ignoreCase)
{
    int stringLen = strlen(s);
    int endLen = strlen(expectedEnd);
    const char *sp;
    int i;

    if (endLen > stringLen) return 0;

    sp = s + stringLen - endLen;
    for (i = 0; i < endLen; ++i)
    {
        if (ignoreCase)
        {
            if (toupper(sp[i]) != toupper(expectedEnd[i])) return 0;
        }
        else
        {
            if (sp[i] != expectedEnd[i]) return 0;
        }
    }

    return 1;
}

SOEXPORT void *
mkd64Alloc(size_t size)
{
    void *alloc = malloc(size);
    if (!alloc)
    {
        perror("Fatal: Could not allocate memory");
        exit(EXIT_FAILURE);
    }
    return alloc;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
