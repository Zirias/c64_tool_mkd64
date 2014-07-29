#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#include <limits.h>
#include <stdlib.h>

#include <mkd64/common.h>
#include <mkd64/debug.h>

#include "../../util.h"
#include "../../mkd64.h"
#include "../../cmdline.h"

#include <string.h>
#include <glob.h>
#include <libgen.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

SOEXPORT void
findFilesInDir(const char *dir, const char *pattern, void *caller,
        FileFoundCallback found)
{
    glob_t glb;
    char **pathvp;
    char *pat = malloc(strlen(dir) + strlen(pattern) + 1);
    
    strcpy(pat, dir);
    strcat(pat, pattern);
    glob(pat, 0, 0, &glb);
    free(pat);

    for (pathvp = glb.gl_pathv; pathvp && *pathvp; ++pathvp)
    {
        found(caller, *pathvp);
    }

    globfree(&glb);
}

SOLOCAL char *
getAppDir(const char *expectedAppNameEnd, void *caller,
        AppNameMismatchCallback mismatch)
{
    const char *exe = Cmdline_exe(Mkd64_cmdline(MKD64));
    char *exefullpath = realpath(exe, 0);
    char *dir = 0;

    if (expectedAppNameEnd)
    {
        if (stringEndsWith(exefullpath, expectedAppNameEnd, 1))
        {
            dir = copyString(dirname(exefullpath));
        }
        else if (mismatch)
        {
            mismatch(caller, exefullpath);
        }
    }
    else
    {
        dir = copyString(dirname(exefullpath));
    }

    free(exefullpath);
    return dir;
}

SOLOCAL int64_t
getFileSize(const FILE *file)
{
    struct stat st;
    if (fstat(file, &st) < 0) return -1;
    return (int64_t) st.st_size;
}

SOLOCAL void *
loadDso(const char *name)
{
    void *dso = dlopen(name, RTLD_NOW);
    if (!dso)
    {
        DBG(dlerror());
    }
    return dso;
}

SOLOCAL void *
getDsoFunction(void *dso, const char *name)
{
    return dlsym(dso, name);
}

SOLOCAL void
unloadDso(void *dso)
{
    dlclose(dso);
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
