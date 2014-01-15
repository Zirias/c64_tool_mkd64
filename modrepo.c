#ifdef WIN32

#include <windows.h>
#include <shlwapi.h>
#define LOAD_MOD(name) LoadLibrary(name)
#define GET_MOD_METHOD(so, name) GetProcAddress(so, name)
#define UNLOAD_MOD(so) FreeLibrary(so)
#define dlerror(x) "Error loading module"

#else

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#include <glob.h>
#include <limits.h>
#include <stdlib.h>
#include <libgen.h>
#include <dlfcn.h>
#define LOAD_MOD(name) dlopen(name, RTLD_NOW)
#define GET_MOD_METHOD(so, name) dlsym(so, name)
#define UNLOAD_MOD(so) dlclose(so)

#endif

#include <mkd64/common.h>
#include <mkd64/debug.h>

#include "modrepo.h"

#include <stdio.h>
#include <string.h>

typedef struct ModInstance ModInstance;

struct ModInstance
{
    ModInstance *next;
    const char *id;
    IModule *mod;
};

typedef struct ModEntry ModEntry;

struct ModEntry
{
    ModEntry *next;
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

struct ModRepo
{
    void *owner;
    ModInstanceCreated callback;
    ModEntry *modules;
    ModInstance *instances;
};

static ModEntry *
findModule(const ModRepo *this, const char *id)
{
    ModEntry *current;

    for (current = this->modules; current; current = current->next)
    {
        if (!strcmp(id, current->id))
        {
            return current;
        }
    }
    return 0;
}

static ModInstance *
findInstance(const ModRepo *this, const char *id)
{
    ModInstance *current;
    
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
appendInstance(ModRepo *this, IModule *instance)
{
    ModInstance *current = malloc(sizeof(ModInstance));
    ModInstance *parent;

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
createInstanceHere(ModRepo *this, ModEntry *entry)
{
    ModEntry *otherMod;
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
endsWith(const char *value, const char *pattern)
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

static int
checkApiVersion(const char *name, const int *ver)
{
    if (ver[0] > API_VER_MAJOR ||
           (ver[0] == API_VER_MAJOR && ver[1] > API_VER_MINOR))
    {
#ifdef API_VER_BETA
        fprintf(stderr, "Warning: module `%s' needs a newer version of mkd64. "
                "(mkd64 API %d.%d (beta), %s API %d.%d)\n", name,
                API_VER_MAJOR, API_VER_MINOR, name, ver[0], ver[1]);
#else
        fprintf(stderr, "Warning: module `%s' needs a newer version of mkd64. "
                "(mkd64 API %d.%d, %s API %d.%d)\n", name,
                API_VER_MAJOR, API_VER_MINOR, name, ver[0], ver[1]);
#endif
        return 0;
    }
#ifdef API_VER_BETA
    else if (ver[0] < API_VER_MAJOR ||
            (ver[0] == API_VER_MAJOR && ver[1] < API_VER_MAJOR))
    {
        fprintf(stderr, "Warning: module `%s' is outdated. "
                "(mkd64 API %d.%d (beta), %s API %d.%d)\n", name,
                API_VER_MAJOR, API_VER_MINOR, name, ver[0], ver[1]);
#else
    else if (ver[0] < API_VER_MAJOR)
    {
        fprintf(stderr, "Warning: module `%s' is outdated. "
                "(mkd64 API %d.%d, %s API %d.%d)\n", name,
                API_VER_MAJOR, API_VER_MINOR, name, ver[0], ver[1]);
#endif
        return 0;
    }
    return 1;
}

static void
findModuleObjects(ModRepo *this, const char *exe)
{
    ModEntry *current = 0;
    ModEntry *next;
    const int *apiVer;
    const char *name;

#ifdef WIN32
    HANDLE findHdl;
    HMODULE modso;
    FARPROC modid, modinst, modopt;
    WIN32_FIND_DATA findData;

    char *modpat = malloc(4096);
    GetModuleFileName(GetModuleHandle(0), modpat, 4096);
    if (!endsWith(modpat, "\\MKD64.EXE"))
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
        name = findData.cFileName;
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
        name = *pathvp;
#endif
        modso = LOAD_MOD(name);
        if (!modso)
        {
            DBG(dlerror());
            continue;
        }

        modopt = GET_MOD_METHOD(modso, "mkd64ApiVersion");
        if (!modopt)
        {
            DBGs1("Not an mkd64 module:", name);
            UNLOAD_MOD(modso);
            continue;
        }

        apiVer = ((const int *(*)(void))((uintptr_t)modopt))();

        if (!checkApiVersion(name, apiVer))
        {
            UNLOAD_MOD(modso);
            continue;
        }

        modid = GET_MOD_METHOD(modso, "id");
        if (!modid)
        {
            fprintf(stderr, "Warning: Erroneous module `%s', missing "
                    "'const char *id()' export.\n", name);
            UNLOAD_MOD(modso);
            continue;
        }

        modinst = GET_MOD_METHOD(modso, "instance");
        if (!modinst)
        {
            fprintf(stderr, "Warning: Erroneous module `%s', missing "
                    "'IModule *instance()' export.\n", name);
            UNLOAD_MOD(modso);
            continue;
        }

        next = malloc(sizeof(ModEntry));
        next->next = 0;
        next->so = modso;
        next->id = ((const char *(*)(void))((uintptr_t)modid))();
        next->instance = (IModule *(*)(void))((uintptr_t)modinst);

        modopt = GET_MOD_METHOD(modso, "depends");
        next->depends = modopt ?
            ((const char **(*)(void))((uintptr_t)modopt))() : 0;

        modopt = GET_MOD_METHOD(modso, "conflicts");
        next->conflicts = modopt ?
            ((const char **(*)(void))((uintptr_t)modopt))() : 0;

        modopt = GET_MOD_METHOD(modso, "help");
        next->help = modopt ? (const char *(*)(void))((uintptr_t)modopt) : 0;

        modopt = GET_MOD_METHOD(modso, "helpFile");
        next->helpFile = modopt ?
            (const char *(*)(void))((uintptr_t)modopt) : 0;

        modopt = GET_MOD_METHOD(modso, "versionInfo");
        next->versionInfo = modopt ?
            (const char *(*)(void))((uintptr_t)modopt) : 0;

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
}

SOLOCAL size_t
ModRepo_objectSize(void)
{
    return sizeof(ModRepo);
}

SOLOCAL ModRepo *
ModRepo_init(ModRepo *this, const char *exe, void *owner,
        ModInstanceCreated callback)
{
    memset(this, 0, sizeof(ModRepo));

    findModuleObjects(this, exe);
    this->owner = owner;
    this->callback = callback;

    return this;
}

SOLOCAL void
ModRepo_done(ModRepo *this)
{
    ModEntry *currentEntry = this->modules;
    ModInstance *currentInstance = this->instances;
    ModEntry *tmpEntry;
    ModInstance *tmpInstance;

    while (currentInstance)
    {
        tmpInstance = currentInstance;
        currentInstance = currentInstance->next;
        tmpInstance->mod->free(tmpInstance->mod);
        free(tmpInstance);
    }

    while (currentEntry)
    {
        tmpEntry = currentEntry;
        currentEntry = currentEntry->next;
        UNLOAD_MOD(tmpEntry->so);
        free(tmpEntry);
    }
}

SOLOCAL void
ModRepo_reloadModules(ModRepo *this)
{
    ModEntry *entry;
    ModInstance *current;

    for (current = this->instances; current; current = current->next)
    {
        current->mod->free(current->mod);
        entry = findModule(this, current->id);
        current->mod = entry->instance();
        this->callback(this->owner, current->mod);
    }
}

SOEXPORT IModule *
ModRepo_moduleInstance(ModRepo *this, const char *id)
{
    ModInstance *found;

    found = findInstance(this, id);
    if (!found)
    {
        ModRepo_createInstance(this, id);
        found = findInstance(this, id);
    }
    if (!found) return 0;
    return found->mod;
}

SOLOCAL int
ModRepo_createInstance(ModRepo *this, const char *id)
{
    ModEntry *found;

    if (findInstance(this, id)) return 1;

    found = findModule(this, id);
    if (!found) return 0;
    return createInstanceHere(this, found);
}

SOLOCAL int
ModRepo_deleteInstance(ModRepo *this, const char *id)
{
    ModInstance *current, *parent;

    if (!this->instances) return 0;

    parent = 0;
    for (current = this->instances;
            current && strcmp(current->id,id) != 0;
            parent = current, current = current->next) {}
    if (current)
    {
        if (parent) parent->next = current->next;
        else this->instances = current->next;
        current->mod->free(current->mod);
        free(current);
        return 1;
    }
    return 0;
}

SOEXPORT int
ModRepo_isActive(const ModRepo *this, const char *id)
{
    return findInstance(this, id) ? 1 : 0;
}

SOLOCAL char *
ModRepo_getHelp(const ModRepo *this, const char *id)
{
    static const char *mainHelpHeader = "* Module `%s':\n\n";
    static const char *noHelp = "  No help available.\n";
    static const char *fileHelpHeader = "\n* File options:\n\n";
    char *helpText;
    size_t helpLen;
    const char *mainHelp, *fileHelp;
    ModEntry *found;

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
ModRepo_getVersionInfo(const ModRepo *this, const char *id)
{
    static const char *versionHeader = "* Module `%s':\n\n";
    static const char *noVersion = "  No version info available.\n";
    char *versionText;
    size_t versionLen;
    const char *versionInfo;
    ModEntry *found;

    found = findModule(this, id);
    if (!found) return 0;
    versionInfo = found->versionInfo ? found->versionInfo() : 0;

    versionLen = strlen(versionHeader) + strlen(id) - 1;

    if (versionInfo) versionLen += strlen(versionInfo);
    else versionLen += strlen(noVersion);

    versionText = malloc(versionLen);
    sprintf(versionText, versionHeader, id);
    if (versionInfo) strcat(versionText, versionInfo);
    else strcat(versionText, noVersion);

    return versionText;
}

SOLOCAL void
ModRepo_allInitImage(const ModRepo *this, Image *image)
{
    ModInstance *current;
    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->initImage)
            current->mod->initImage(current->mod, image);
    }
}

SOLOCAL int
ModRepo_allGlobalOption(const ModRepo *this, char opt, const char *arg)
{
    ModInstance *current;
    int handled = 0;

    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->globalOption)
        {
            if (current->mod->globalOption(current->mod, opt, arg))
            {
                handled = 1;
            }
        }
    }
    return handled;
}

SOLOCAL int
ModRepo_allFileOption(const ModRepo *this, DiskFile *file,
        char opt, const char *arg)
{
    ModInstance *current;
    int handled = 0;

    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->fileOption)
        {
            if (current->mod->fileOption(current->mod, file, opt, arg))
            {
                handled = 1;
            }
        }
    }
    return handled;
}

SOLOCAL Track *
ModRepo_firstGetTrack(const ModRepo *this, int track)
{
    Track *t = 0;
    ModInstance *current;

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
ModRepo_allFileWritten(const ModRepo *this,
        DiskFile *file, const BlockPosition *start)
{
    ModInstance *current;
    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->fileWritten)
            current->mod->fileWritten(current->mod, file, start);
    }
}

SOLOCAL void
ModRepo_allStatusChanged(const ModRepo *this, const BlockPosition *pos)
{
    ModInstance *current;
    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->statusChanged)
            current->mod->statusChanged(current->mod, pos);
    }
}

SOLOCAL void
ModRepo_allImageComplete(const ModRepo *this)
{
    ModInstance *current;
    for (current = this->instances; current; current = current->next)
    {
        if (current->mod->imageComplete)
            current->mod->imageComplete(current->mod);
    }
}

SOLOCAL const char *
ModRepo_nextAvailableModule(const ModRepo *this, const char *id)
{
    ModEntry *found;
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
ModRepo_nextLoadedModule(const ModRepo *this, const char *id)
{
    ModInstance *found;
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
