#include <mkd64/common.h>
#include <mkd64/debug.h>

#include "../../util.h"

#include <windows.h>
#include <shlwapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

SOEXPORT void
findFilesInDir(const char *dir, const char *pattern, void *caller,
        FileFoundCallback found)
{
    HANDLE findHdl;
    WIN32_FIND_DATA findData;
    char *pat = malloc(strlen(dir) + strlen(pattern) + 1);

    strcpy(pat, dir);
    strcat(pat, pattern);
    
    findHdl = FindFirstFile(pat, &findData);
    if (findHdl == INVALID_HANDLE_VALUE)
    {
        free(pat);
        return;
    }

    do
    {
        found(caller, findData.cFileName);
    } while (FindNextFile(findHdl, &findData) != 0);

    FindClose(findHdl);
    free(pat);
}

SOLOCAL char *
getAppDir(const char *expectedAppNameEnd, void *caller,
        AppNameMismatchCallback mismatch)
{
    char *path = malloc(4096);
    GetModuleFileName(GetModuleHandle(0), path, 4096);

    if (expectedAppNameEnd)
    {
        if (stringEndsWith(path, expectedAppNameEnd, 1))
        {
            PathRemoveFileSpec(path);
            return path;
        }
        else if (mismatch)
        {
            mismatch(caller, path);
        }
    }
    else
    {
        PathRemoveFileSpec(path);
        return path;
    }

    free(path);
    return 0;
}

#ifdef DEBUG
void
printErrorMessage(int id, ...)
{
    char buf[4096];
    va_list vl;

    int error = GetLastError();
    va_start(vl, id);
    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0,
            buf, 4096, &vl))
    {
        DBGn(buf);
    }
    va_end(vl);
}
#endif

SOLOCAL void *
loadDso(const char *name)
{
    void *dso;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    dso = LoadLibrary(name);
#ifdef DEBUG
    if (!dso)
    {
        printErrorMessage(GetLastError(), name);
    }
#endif
    SetErrorMode(0);
    return dso;
}

SOLOCAL void *
getDsoFunction(void *dso, const char *name)
{
    return (void *)(uintptr_t)GetProcAddress(dso, name);
}

SOLOCAL void
unloadDso(void *dso)
{
    FreeLibrary(dso);
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
