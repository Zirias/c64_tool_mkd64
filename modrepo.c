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

struct modinstance;
typedef struct modinstance Modinstance;

struct modinstance
{
    Modinstance *next;
    const char *id;
    IModule *mod;
};

struct modentry;
typedef struct modentry Modentry;

struct modentry
{
    Modentry *next;
    void *so;
    const char *id;
    IModule *(*instance)(void);
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
    Modentry *modules;
    Modinstance *instances;
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

static Modinstance *
findInstance(Modrepo *this, const char *id)
{
    Modinstance *current;
    
    for (current = this->instances; current; current = current->next)
    {
        if (!strcmp(id, current->id))
        {
            return current;
        }
    }
    return 0;
}

static void
appendInstance(Modrepo *this, IModule *instance)
{
    Modinstance *current = malloc(sizeof(Modinstance));
    Modinstance *parent;

    current->id = instance->id();
    current->mod = instance;
    current->next = 0;

    if (this->instances)
    {
        for (parent = this->instances; parent->next; parent = parent->next) {}
        parent->next = current;
    }
    else
    {
        this->instances = current;
    }
}

static int
createInstanceHere(Modrepo *this, Modentry *entry)
{
    Modentry *otherMod;
    const char **otherModId;
    IModule *created;

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
            if (findInstance(this, *otherModId))
            {
                fprintf(stderr,
"Error: cannot load module `%s' because it conflicts with already loaded\n"
"       module `%s'.\n", entry->id, *otherModId);
                return 0;
            }
            otherMod = findModule(this, *otherModId);
            if (otherMod)
            {
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
            if (!findInstance(this, *otherModId))
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

    created = entry->instance();
    appendInstance(this, created);
    this->callback(this->owner, created);
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
    Modentry *current = 0;
    Modentry *next;

#ifdef WIN32
    HANDLE findHdl;
    HMODULE modso;
    FARPROC modid, modinst, modopt;
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
    void *modso, *modid, *modinst, *modopt;

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

        next = malloc(sizeof(Modentry));
        next->next = 0;
        next->so = modso;
        next->id = ((const char *(*)(void))modid)();
        next->instance = (IModule *(*)(void))modinst;

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

#ifdef WIN32
    } while (FindNextFile(findHdl, &findData) != 0);

    SetErrorMode(0);
    FindClose(findHdl);
    free(modpat);
#else
    }

    globfree(&glb);
#endif
    this->owner = owner;
    this->callback = callback;

    return this;
}

SOLOCAL void
modrepo_delete(Modrepo *this)
{
    Modentry *currentEntry = this->modules;
    Modinstance *currentInstance = this->instances;
    Modentry *tmpEntry;
    Modinstance *tmpInstance;

    while (currentInstance)
    {
        tmpInstance = currentInstance;
        currentInstance = currentInstance->next;
        tmpInstance->mod->delete(tmpInstance->mod);
        free(tmpInstance);
    }

    while (currentEntry)
    {
        tmpEntry = currentEntry;
        currentEntry = currentEntry->next;
        UNLOAD_MOD(tmpEntry->so);
        free(tmpEntry);
    }
    free(this);
}

SOEXPORT IModule *
modrepo_moduleInstance(Modrepo *this, const char *id)
{
    Modinstance *found;

    found = findInstance(this, id);
    if (!found)
    {
        modrepo_createInstance(this, id);
        found = findInstance(this, id);
    }
    if (!found) return 0;
    return found->mod;
}

SOLOCAL int
modrepo_createInstance(Modrepo *this, const char *id)
{
    Modentry *found;

    if (findInstance(this, id)) return 1;

    found = findModule(this, id);
    if (!found) return 0;
    return createInstanceHere(this, found);
}

SOLOCAL int
modrepo_deleteInstance(Modrepo *this, const char *id)
{
    Modinstance *current, *parent;

    if (!this->instances) return 0;

    parent = 0;
    for (current = this->instances;
            current && strcmp(current->id,id) != 0;
            parent = current, current = current->next) {}
    if (current)
    {
        if (parent) parent->next = current->next;
        else this->instances = current->next;
        current->mod->delete(current->mod);
        free(current);
        return 1;
    }
    return 0;
}

SOEXPORT int
modrepo_isActive(Modrepo *this, const char *id)
{
    return findInstance(this, id) ? 1 : 0;
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
    Modinstance *current;
    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->initImage)
            current->mod->initImage(current->mod, image);
    }
}

SOLOCAL void
modrepo_allGlobalOption(Modrepo *this, char opt, const char *arg)
{
    Modinstance *current;
    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->globalOption)
            current->mod->globalOption(current->mod, opt, arg);
    }
}

SOLOCAL void
modrepo_allFileOption(Modrepo *this, Diskfile *file, char opt, const char *arg)
{
    Modinstance *current;
    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->fileOption)
            current->mod->fileOption(current->mod, file, opt, arg);
    }
}

SOLOCAL Track *
modrepo_firstGetTrack(Modrepo *this, int track)
{
    Track *t;
    Modinstance *current;

    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->getTrack)
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
    Modinstance *current;
    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->fileWritten)
            current->mod->fileWritten(current->mod, file, start);
    }
}

SOLOCAL void
modrepo_allStatusChanged(Modrepo *this, const BlockPosition *pos)
{
    Modinstance *current;
    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->statusChanged)
            current->mod->statusChanged(current->mod, pos);
    }
}

SOLOCAL const char *
modrepo_nextAvailableModule(Modrepo *this, const char *id)
{
    Modentry *found;
    if (!id)
    {
        if (this->modules) return this->modules->id;
        else return 0;
    }
    found = findModule(this, id);
    if (!found) return 0;
    if (found->next) return found->next->id;
    return 0;
}

SOLOCAL const char *
modrepo_nextLoadedModule(Modrepo *this, const char *id)
{
    Modinstance *found;
    if (!id)
    {
        if (this->instances) return this->instances->id;
        else return 0;
    }
    found = findInstance(this, id);
    if (!found) return 0;
    if (found->next) return found->next->id;
    return 0;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
