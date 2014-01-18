#include <mkd64/common.h>
#include <mkd64/debug.h>
#include "util.h"
#include "platform/defs.h"

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
findModule(const ModRepo *self, const char *id)
{
    ModEntry *current;

    for (current = self->modules; current; current = current->next)
    {
        if (!strcmp(id, current->id))
        {
            return current;
        }
    }
    return 0;
}

static ModInstance *
findInstance(const ModRepo *self, const char *id)
{
    ModInstance *current;
    
    for (current = self->instances; current; current = current->next)
    {
        if (!strcmp(id, current->id))
        {
            return current;
        }
    }
    return 0;
}

static void
appendInstance(ModRepo *self, IModule *instance)
{
    ModInstance *current = malloc(sizeof(ModInstance));
    ModInstance *parent;

    current->id = instance->id();
    current->mod = instance;
    current->next = 0;

    if (self->instances)
    {
        for (parent = self->instances; parent->next; parent = parent->next) {}
        parent->next = current;
    }
    else
    {
        self->instances = current;
    }
}

static int
createInstanceHere(ModRepo *self, ModEntry *entry)
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
            if (findInstance(self, *otherModId))
            {
                fprintf(stderr,
"Error: cannot load module `%s' because it conflicts with already loaded\n"
"       module `%s'.\n", entry->id, *otherModId);
                return 0;
            }
            otherMod = findModule(self, *otherModId);
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
            otherMod = findModule(self, *otherModId);
            if (!otherMod)
            {
                fprintf(stderr,
"Error: cannot load module `%s' because it depends on `%s' which is not\n"
"       available.\n", entry->id, *otherModId);
                return 0;
            }
            if (!findInstance(self, *otherModId))
            {
                fprintf(stderr, "Info: loading module `%s' "
                        "because `%s' depends on it.\n",
                        otherMod->id, entry->id);
                if (!createInstanceHere(self, otherMod))
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
    appendInstance(self, created);
    self->callback(self->owner, created);
    return 1;
}

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
addModuleFile(void *caller, const char *filename)
{
    ModRepo *self = caller;
    ModEntry *current, *next;
    void *modso, *modid, *modinst, *modopt;
    const int *apiVer;

    modso = loadDso(filename);
    if (!modso) return;

    modopt = getDsoFunction(modso, "mkd64ApiVersion");
    if (!modopt)
    {
        DBGs1("Not an mkd64 module:", filename);
        unloadDso(modso);
        return;
    }

    apiVer = ((const int *(*)(void))((uintptr_t)modopt))();

    if (!checkApiVersion(filename, apiVer))
    {
        unloadDso(modso);
        return;
    }

    modid = getDsoFunction(modso, "id");
    if (!modid)
    {
        fprintf(stderr, "Warning: Erroneous module `%s', missing "
                "'const char *id()' export.\n", filename);
        unloadDso(modso);
        return;
    }

    modinst = getDsoFunction(modso, "instance");
    if (!modinst)
    {
        fprintf(stderr, "Warning: Erroneous module `%s', missing "
                "'IModule *instance()' export.\n", filename);
        unloadDso(modso);
        return;
    }

    next = malloc(sizeof(ModEntry));
    next->next = 0;
    next->so = modso;
    next->id = ((const char *(*)(void))((uintptr_t)modid))();
    next->instance = (IModule *(*)(void))((uintptr_t)modinst);

    modopt = getDsoFunction(modso, "depends");
    next->depends = modopt ?
        ((const char **(*)(void))((uintptr_t)modopt))() : 0;

    modopt = getDsoFunction(modso, "conflicts");
    next->conflicts = modopt ?
        ((const char **(*)(void))((uintptr_t)modopt))() : 0;

    modopt = getDsoFunction(modso, "help");
    next->help = modopt ? (const char *(*)(void))((uintptr_t)modopt) : 0;

    modopt = getDsoFunction(modso, "helpFile");
    next->helpFile = modopt ?
        (const char *(*)(void))((uintptr_t)modopt) : 0;

    modopt = getDsoFunction(modso, "versionInfo");
    next->versionInfo = modopt ?
        (const char *(*)(void))((uintptr_t)modopt) : 0;

    next->conflicted = 0;

    if (self->modules)
    {
        for (current = self->modules; current->next; current = current->next);
        current->next = next;
    }
    else
    {
        self->modules = next;
    }

    DBGs1("Found module:", next->id);
}

#ifdef APP_FILENAME
static void appNameMismatch(void *caller, const char *appName)
{
    (void) caller; /* unused */
    (void) appName; /* unused */

    fputs("\n\n********\n"
            "WARNING: Loading of modules will not work!\n"
            "         The executable must be named `" APP_FILENAME "' to load "
            "modules.\n********\n\n", stderr);
    return;
}
#endif

SOLOCAL size_t
ModRepo_objectSize(void)
{
    return sizeof(ModRepo);
}

void
loadModuleFiles(ModRepo *self)
{
#ifdef MODDIR
    findFilesInDir(MODDIR, FINDPAT_MODULES, self, &addModuleFile);
#else
#ifdef APP_FILENAME
    char *appDir = getAppDir(PATH_SEP APP_FILENAME, self, &appNameMismatch);
#else
    char *appDir = getAppDir(0, 0, 0);
#endif
    if (appDir)
    {
        findFilesInDir(appDir, FINDPAT_MODULES, self, &addModuleFile);
        free(appDir);
    }
#endif
}

SOLOCAL ModRepo *
ModRepo_init(ModRepo *self, void *owner, ModInstanceCreated callback)
{
    memset(self, 0, sizeof(ModRepo));

    loadModuleFiles(self);
    self->owner = owner;
    self->callback = callback;

    return self;
}

SOLOCAL void
ModRepo_done(ModRepo *self)
{
    ModEntry *currentEntry = self->modules;
    ModInstance *currentInstance = self->instances;
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
        unloadDso(tmpEntry->so);
        free(tmpEntry);
    }
}

SOLOCAL void
ModRepo_reloadModules(ModRepo *self)
{
    ModEntry *entry;
    ModInstance *current;

    for (current = self->instances; current; current = current->next)
    {
        current->mod->free(current->mod);
        entry = findModule(self, current->id);
        current->mod = entry->instance();
        self->callback(self->owner, current->mod);
    }
}

SOEXPORT IModule *
ModRepo_moduleInstance(ModRepo *self, const char *id)
{
    ModInstance *found;

    found = findInstance(self, id);
    if (!found)
    {
        ModRepo_createInstance(self, id);
        found = findInstance(self, id);
    }
    if (!found) return 0;
    return found->mod;
}

SOLOCAL int
ModRepo_createInstance(ModRepo *self, const char *id)
{
    ModEntry *found;

    if (findInstance(self, id)) return 1;

    found = findModule(self, id);
    if (!found) return 0;
    return createInstanceHere(self, found);
}

SOLOCAL int
ModRepo_deleteInstance(ModRepo *self, const char *id)
{
    ModInstance *current, *parent;

    if (!self->instances) return 0;

    parent = 0;
    for (current = self->instances;
            current && strcmp(current->id,id) != 0;
            parent = current, current = current->next) {}
    if (current)
    {
        if (parent) parent->next = current->next;
        else self->instances = current->next;
        current->mod->free(current->mod);
        free(current);
        return 1;
    }
    return 0;
}

SOEXPORT int
ModRepo_isActive(const ModRepo *self, const char *id)
{
    return findInstance(self, id) ? 1 : 0;
}

SOLOCAL char *
ModRepo_getHelp(const ModRepo *self, const char *id)
{
    static const char *mainHelpHeader = "* Module `%s':\n\n";
    static const char *noHelp = "  No help available.\n";
    static const char *fileHelpHeader = "\n* File options:\n\n";
    char *helpText;
    size_t helpLen;
    const char *mainHelp, *fileHelp;
    ModEntry *found;

    found = findModule(self, id);
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
ModRepo_getVersionInfo(const ModRepo *self, const char *id)
{
    static const char *versionHeader = "* Module `%s':\n\n";
    static const char *noVersion = "  No version info available.\n";
    char *versionText;
    size_t versionLen;
    const char *versionInfo;
    ModEntry *found;

    found = findModule(self, id);
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
ModRepo_allInitImage(const ModRepo *self, Image *image)
{
    ModInstance *current;
    for (current = self->instances; current; current = current->next)
    {
        if (current->mod->initImage)
            current->mod->initImage(current->mod, image);
    }
}

SOLOCAL int
ModRepo_allGlobalOption(const ModRepo *self, char opt, const char *arg)
{
    ModInstance *current;
    int handled = 0;

    for (current = self->instances; current; current = current->next)
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
ModRepo_allFileOption(const ModRepo *self, DiskFile *file,
        char opt, const char *arg)
{
    ModInstance *current;
    int handled = 0;

    for (current = self->instances; current; current = current->next)
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
ModRepo_firstGetTrack(const ModRepo *self, int track)
{
    Track *t = 0;
    ModInstance *current;

    for (current = self->instances; current; current = current->next)
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
ModRepo_allFileWritten(const ModRepo *self,
        DiskFile *file, const BlockPosition *start)
{
    ModInstance *current;
    for (current = self->instances; current; current = current->next)
    {
        if (current->mod->fileWritten)
            current->mod->fileWritten(current->mod, file, start);
    }
}

SOLOCAL void
ModRepo_allStatusChanged(const ModRepo *self, const BlockPosition *pos)
{
    ModInstance *current;
    for (current = self->instances; current; current = current->next)
    {
        if (current->mod->statusChanged)
            current->mod->statusChanged(current->mod, pos);
    }
}

SOLOCAL void
ModRepo_allImageComplete(const ModRepo *self)
{
    ModInstance *current;
    for (current = self->instances; current; current = current->next)
    {
        if (current->mod->imageComplete)
            current->mod->imageComplete(current->mod);
    }
}

SOLOCAL const char *
ModRepo_nextAvailableModule(const ModRepo *self, const char *id)
{
    ModEntry *found;
    if (!id)
    {
        if (self->modules) return self->modules->id;
        else return 0;
    }
    found = findModule(self, id);
    if (!found) return 0;
    if (found->next) return found->next->id;
    return 0;
}

SOLOCAL const char *
ModRepo_nextLoadedModule(const ModRepo *self, const char *id)
{
    ModInstance *found;
    if (!id)
    {
        if (self->instances) return self->instances->id;
        else return 0;
    }
    found = findInstance(self, id);
    if (!found) return 0;
    if (found->next) return found->next->id;
    return 0;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
