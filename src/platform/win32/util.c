#undef __STRICT_ANSI__
#include <mkd64/common.h>
#include <mkd64/debug.h>

#include "../../util.h"

#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <shlwapi.h>
#include <io.h>

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

SOEXPORT int64_t
getFileSize(const FILE *file)
{
    HANDLE fileHdl;
    LARGE_INTEGER size;

    fileHdl = (HANDLE) _get_osfhandle(_fileno(file));
    if (fileHdl == INVALID_HANDLE_VALUE) return -1;
    if (!GetFileSizeEx(fileHdl, &size)) return -1;
    return (int64_t) size.QuadPart;
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

    va_start(vl, id);
    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, id, 0,
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
