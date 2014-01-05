#include <mkd64/common.h>
#include <mkd64/debug.h>

#include "modrepo.h"

#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#include <shlwapi.h>
#define GET_MOD_METHOD(so, name) GetProcAddress(so, name)
#define UNLOAD_MOD(so) FreeLibrary(so)
#define dlerror(x) "Error loading module"
#else
#include <glob.h>
#include <limits.h>
#include <stdlib.h>
#include <libgen.h>
#include <dlfcn.h>
#define GET_MOD_METHOD(so, name) dlsym(so, name)
#define UNLOAD_MOD(so) dlclose(so)
#endif

struct modentry;
typedef struct modentry Modentry;

struct modentry
{
    Modentry *next;
    void *so;
    IModule *mod;
    const char *id;
    IModule *(*instance)(void);
    void (*delete)(IModule *instance);
    const char **depends;
    const char **conflicts;
    const char *(*help)(void);
    const char *(*helpFile)(void);
    const char *(*versionInfo)(void);
    int conflicted;
};

struct modrepo
{
    void *owner;
    ModInstanceCreated callback;
    const char *availableModules[128];
    Modentry *modules;
};

static Modentry *
findModule(Modrepo *this, const char *id)
{
    Modentry *current;

    for (current = this->modules; current; current = current->next)
    {
        if (!strcmp(id, current->id))
        {
            return current;
        }
    }
    return 0;
}

static int
createInstanceHere(Modrepo *this, Modentry *entry)
{
    Modentry *otherMod;
    const char **otherModId;

    if (entry->conflicted)
    {
        fprintf(stderr,
"Error: cannot load module `%s' because an already loaded module conflicts\n"
"       with it.\n", entry->id);
        return 0;
    }

    if (entry->conflicts)
    {
        for (otherModId = entry->conflicts; *otherModId; ++otherModId)
        {
            otherMod = findModule(this, *otherModId);
            if (otherMod)
            {
                if (otherMod->mod)
                {
                    fprintf(stderr,
"Error: cannot load module `%s' because it conflicts with already loaded\n"
"       module `%s'.\n", entry->id, otherMod->id);
                    return 0;
                }
                otherMod->conflicted = 1;
            }
        }
    }

    if (entry->depends)
    {
        for (otherModId = entry->depends; *otherModId; ++otherModId)
        {
            otherMod = findModule(this, *otherModId);
            if (!otherMod)
            {
                fprintf(stderr,
"Error: cannot load module `%s' because it depends on `%s' which is not\n"
"       available.\n", entry->id, *otherModId);
                return 0;
            }
            if (!otherMod->mod)
            {
                fprintf(stderr, "Info: loading module `%s' "
                        "because `%s' depends on it.\n",
                        otherMod->id, entry->id);
                if (!createInstanceHere(this, otherMod))
                {
                    fprintf(stderr,
"Error: cannot load module `%s' because its dependency `%s' failed to load.",
                            entry->id, otherMod->id);
                    return 0;
                }
            }
        }
    }

    entry->mod = entry->instance();
    this->callback(this->owner, entry->mod);
    return 1;
}

#ifdef WIN32
#include <ctype.h>

static int
_endsWith(const char *value, const char *pattern)
{
    int valueLen = strlen(value);
    int patternLen = strlen(pattern);
    const char *valuePtr;
    int i;

    if (patternLen > valueLen) return 0;

    valuePtr = value + valueLen - patternLen;
    for (i = 0; i < patternLen; ++i)
    {
        if (toupper(valuePtr[i]) != toupper(pattern[i])) return 0;
    }

    return 1;
}
#endif

SOLOCAL Modrepo *
modrepo_new(const char *exe, void *owner, ModInstanceCreated callback)
{
    Modrepo *this = calloc(1, sizeof(Modrepo));
    int i = 0;
    Modentry *current = 0;
    Modentry *next;

#ifdef WIN32
    HANDLE findHdl;
    HMODULE modso;
    FARPROC modid, modinst, moddel, modopt;
    WIN32_FIND_DATA findData;

    char *modpat = malloc(4096);
    GetModuleFileName(GetModuleHandle(0), modpat, 4096);
    if (!_endsWith(modpat, "\\MKD64.EXE"))
    {
        fputs("\n\n********\n"
                "WARNING: Loading of modules will not work!\n"
                "         The executable must be named `mkd64.exe' to load "
                "modules.\n********\n\n", stderr);
        free(modpat);
        return 0;
    }
#ifdef MODDIR
    strcpy(modpat, MODDIR);
#else
    PathRemoveFileSpec(modpat);
#endif
    strcat(modpat, "\\*.dll");

    findHdl = FindFirstFile(modpat, &findData);
    if (findHdl == INVALID_HANDLE_VALUE)
    {
        free(modpat);
        return this;
    }

    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    do
    {
        modso = LoadLibrary(findData.cFileName);
#else
    glob_t glb;
    char **pathvp;
    char *modpat;
    void *modso, *modid, *modinst, *moddel, *modopt;

#ifdef MODDIR
    modpat = malloc(strlen(MODDIR) + 6);
    strcpy(modpat, MODDIR);
#else
    char *exefullpath = realpath(exe, 0);
    char *dir = dirname(exefullpath);
    modpat = malloc(strlen(dir) + 6);
    strcpy(modpat, dir);
#endif
    strcat(modpat, "/*.so");
    glob(modpat, 0, 0, &glb);
    free(modpat);
#ifndef MODDIR
    free(exefullpath);
#endif

    for (pathvp = glb.gl_pathv; pathvp && *pathvp; ++pathvp)
    {
        modso = dlopen(*pathvp, RTLD_NOW);
#endif
        if (!modso)
        {
            DBG(dlerror());
            continue;
        }

        modid = GET_MOD_METHOD(modso, "id");
        if (!modid)
        {
            UNLOAD_MOD(modso);
            continue;
        }

        modinst = GET_MOD_METHOD(modso, "instance");
        if (!modinst)
        {
            UNLOAD_MOD(modso);
            continue;
        }

        moddel = GET_MOD_METHOD(modso, "delete");
        if (!moddel)
        {
            UNLOAD_MOD(modso);
            continue;
        }

        next = malloc(sizeof(Modrepo));
        next->next = 0;
        next->so = modso;
        next->mod = 0;
        next->id = ((const char *(*)(void))modid)();
        next->instance = (IModule *(*)(void))modinst;
        next->delete = (void (*)(IModule *))moddel;

        modopt = GET_MOD_METHOD(modso, "depends");
        next->depends = modopt ? ((const char **(*)(void))modopt)() : 0;

        modopt = GET_MOD_METHOD(modso, "conflicts");
        next->conflicts = modopt ? ((const char **(*)(void))modopt)() : 0;

        modopt = GET_MOD_METHOD(modso, "help");
        next->help = modopt ? (const char *(*)(void))modopt : 0;

        modopt = GET_MOD_METHOD(modso, "helpFile");
        next->helpFile = modopt ? (const char *(*)(void))modopt : 0;

        modopt = GET_MOD_METHOD(modso, "versionInfo");
        next->versionInfo = modopt ? (const char *(*)(void))modopt : 0;

        next->conflicted = 0;

        if (current)
        {
            current->next = next;
        }
        else
        {
            this->modules = next;
        }
        current = next;

        DBGs1("Found module:", current->id);
        this->availableModules[i++] = current->id;
        if (i == 128) break;

#ifdef WIN32
    } while (FindNextFile(findHdl, &findData) != 0);

    SetErrorMode(0);
    FindClose(findHdl);
    free(modpat);
#else
    }

    globfree(&glb);
#endif
    this->availableModules[i] = 0;
    this->owner = owner;
    this->callback = callback;

    return this;
}

SOLOCAL void
modrepo_delete(Modrepo *this)
{
    Modentry *current = this->modules;
    Modentry *tmp;

    while (current)
    {
        tmp = current;
        current = current->next;
        if (tmp->mod) tmp->delete(tmp->mod);
        UNLOAD_MOD(tmp->so);
        free(tmp);
    }
    free(this);
}

SOEXPORT IModule *
modrepo_moduleInstance(Modrepo *this, const char *id)
{
    Modentry *found;

    found = findModule(this, id);
    if (!found) return 0;
    if (!found->mod) createInstanceHere(this, found);
    return found->mod;
}

SOLOCAL int
modrepo_createInstance(Modrepo *this, const char *id)
{
    Modentry *found;

    found = findModule(this, id);
    if (!found) return 0;
    if (found->mod) return 1;
    createInstanceHere(this, found);
    return found->mod ? 1 : 0;
}

SOLOCAL int
modrepo_deleteInstance(Modrepo *this, const char *id)
{
    Modentry *found;

    found = findModule(this, id);
    if (!found) return 0;
    found->delete(found->mod);
    found->mod = 0;
    return 1;
}

SOEXPORT int
modrepo_isActive(Modrepo *this, const char *id)
{
    Modentry *found;

    found = findModule(this, id);
    return (found && found->mod) ? 1 : 0;
}

SOLOCAL char *
modrepo_getHelp(Modrepo *this, const char *id)
{
    static const char *mainHelpHeader = "* Module `%s':\n\n";
    static const char *noHelp = "  No help available.\n";
    static const char *fileHelpHeader = "\n* File options:\n\n";
    char *helpText;
    size_t helpLen;
    const char *mainHelp, *fileHelp;
    Modentry *found;

    found = findModule(this, id);
    if (!found) return 0;
    mainHelp = found->help ? found->help() : 0;
    fileHelp = found->helpFile ? found->helpFile() : 0;

    helpLen = strlen(mainHelpHeader) + strlen(id) - 1;

    if (mainHelp) helpLen += strlen(mainHelp);
    if (fileHelp) helpLen += strlen(fileHelpHeader) + strlen(fileHelp);
    if (!mainHelp && !fileHelp) helpLen += strlen(noHelp);

    helpText = malloc(helpLen);
    sprintf(helpText, mainHelpHeader, id);
    if (mainHelp) strcat(helpText, mainHelp);
    if (fileHelp)
    {
        strcat(helpText, fileHelpHeader);
        strcat(helpText, fileHelp);
    }
    if (!mainHelp && !fileHelp) strcat(helpText, noHelp);

    return helpText;
}

SOLOCAL char *
modrepo_getVersionInfo(Modrepo *this, const char *id)
{
    static const char *versionHeader = "* Module `%s':\n\n";
    static const char *noVersion = "  No version info available.\n";
    char *versionText;
    size_t versionLen;
    const char *versionInfo;
    Modentry *found;

    found = findModule(this, id);
    if (!found) return 0;
    versionInfo = found->versionInfo ? found->versionInfo() : 0;

    versionLen = strlen(versionInfo) + strlen(id) - 1;

    if (versionInfo) versionLen += strlen(versionInfo);
    else versionLen += strlen(noVersion);

    versionText = malloc(versionLen);
    sprintf(versionText, versionHeader, id);
    if (versionInfo) strcat(versionText, versionInfo);
    else strcat(versionText, noVersion);

    return versionText;
}

SOLOCAL void
modrepo_allInitImage(Modrepo *this, Image *image)
{
    Modentry *current;
    for (current = this->modules; current; current = current->next)
    {
        if (current->mod && current->mod->initImage)
            current->mod->initImage(current->mod, image);
    }
}

SOLOCAL void
modrepo_allGlobalOption(Modrepo *this, char opt, const char *arg)
{
    Modentry *current;
    for (current = this->modules; current; current = current->next)
    {
        if (current->mod && current->mod->globalOption)
            current->mod->globalOption(current->mod, opt, arg);
    }
}

SOLOCAL void
modrepo_allFileOption(Modrepo *this, Diskfile *file, char opt, const char *arg)
{
    Modentry *current;
    for (current = this->modules; current; current = current->next)
    {
        if (current->mod && current->mod->fileOption)
            current->mod->fileOption(current->mod, file, opt, arg);
    }
}

SOLOCAL Track *
modrepo_firstGetTrack(Modrepo *this, int track)
{
    Track *t;
    Modentry *current;

    for (current = this->modules; current; current = current->next)
    {
        if (current->mod && current->mod->getTrack)
        {
            t = current->mod->getTrack(current->mod, track);
            if (t) break;
        }
    }

    return t;
}

SOLOCAL void
modrepo_allFileWritten(Modrepo *this,
        Diskfile *file, const BlockPosition *start)
{
    Modentry *current;
    for (current = this->modules; current; current = current->next)
    {
        if (current->mod && current->mod->fileWritten)
            current->mod->fileWritten(current->mod, file, start);
    }
}

SOLOCAL void
modrepo_allStatusChanged(Modrepo *this, const BlockPosition *pos)
{
    Modentry *current;
    for (current = this->modules; current; current = current->next)
    {
        if (current->mod && current->mod->statusChanged)
            current->mod->statusChanged(current->mod, pos);
    }
}

SOLOCAL const char * const *
modrepo_foundModules(const Modrepo *this)
{
    return this->availableModules;
}
/* vim: et:si:ts=4:sts=4:sw=4
*/
