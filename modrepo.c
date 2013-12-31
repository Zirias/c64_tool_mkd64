
#include "modrepo.h"

#include <string.h>
#ifdef WIN32
#include <windows.h>
#include <shlwapi.h>
#define GET_MOD_METHOD(so, name) GetProcAddress(so, name)
#define UNLOAD_MOD(so) FreeLibrary(so)
#else
#include <glob.h>
#include <limits.h>
#include <stdlib.h>
#include <libgen.h>
#include <dlfcn.h>
#define GET_MOD_METHOD(so, name) dlsym(so, name)
#define UNLOAD_MOD(so) dlclose(so)
#endif

struct modrepo
{
    Modrepo *next;
    void *so;
    IModule *mod;
    const char *id;
    IModule *(*instance)(void);
    void (*delete)(IModule *instance);
    const char **depends;
    const char **conflicts;
    const char *(*help)(void);
    const char *(*helpFile)(void);
};

static Modrepo *
findModule(Modrepo *this, const char *id)
{
    Modrepo *current;

    for (current = this; current; current = current->next)
    {
        if (!strcmp(id, current->id))
        {
            return current;
        }
    }
    return 0;
}

static int
createInstanceHere(Modrepo *entry)
{
    /* TODO: check dependencies */
    entry->mod = entry->instance();
    return 1;
}

Modrepo *
modrepo_new(const char *exe)
{
    Modrepo *this = 0;
    Modrepo *current = 0;
    Modrepo *next;

#ifdef WIN32
    HANDLE findHdl;
    HMODULE modso;
    FARPROC modid, modinst, moddel, modopt;
    WIN32_FIND_DATA findData;

    char *modpat = malloc(4096);
    GetModuleFileName(GetModuleHandle(0), modpat, 4096);
    PathRemoveFileSpec(modpat);
    strcat(modpat, "\\*.dll");

    findHdl = FindFirstFile(modpat, &findData);
    if (findHdl == INVALID_HANDLE_VALUE)
    {
        free(modpat);
        return this;
    }

    do
    {
        modso = LoadLibrary(findData.cFileName);
#else
    glob_t glb;
    char **pathvp;
    char *modpat;
    void *modso, *modid, *modinst, *moddel, *modopt;

    char *exefullpath = realpath(exe, 0);
    char *dir = dirname(exefullpath);
    modpat = malloc(strlen(dir) + 6);
    strcpy(modpat, dir);
    strcat(modpat, "/*.so");
    glob(modpat, 0, 0, &glb);
    free(modpat);
    free(exefullpath);

    for (pathvp = glb.gl_pathv; pathvp && *pathvp; ++pathvp)
    {
        modso = dlopen(*pathvp, RTLD_NOW);
#endif
        if (!modso) continue;

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

        if (current)
        {
            current->next = next;
        }
        else
        {
            this = next;
        }
        current = next;

        fprintf(stderr, "Found module: %s\n", current->id);

#ifdef WIN32
    } while (FindNextFile(findHdl, &findData) != 0);

    FindClose(findHdl);
    free(modpat);
#else
    }

    globfree(&glb);
#endif

    return this;
}

void
modrepo_delete(Modrepo *this)
{
    Modrepo *current = this;
    Modrepo *tmp;

    while (current)
    {
        tmp = current;
        current = current->next;
        tmp->delete(tmp->mod);
        UNLOAD_MOD(tmp->so);
        free(tmp);
    }
}

IModule *
modrepo_moduleInstance(Modrepo *this, const char *id)
{
    Modrepo *found;

    if (!this) return 0;
    found = findModule(this, id);
    if (!found) return 0;
    if (!found->mod) createInstanceHere(found);
    return found->mod;
}

int
modrepo_createInstance(Modrepo *this, const char *id)
{
    Modrepo *found;

    if (!this) return 0;
    found = findModule(this, id);
    if (!found) return 0;
    if (found->mod) return 1;
    createInstanceHere(found);
    return found->mod ? 1 : 0;
}

int
modrepo_deleteInstance(Modrepo *this, const char *id)
{
    Modrepo *found;

    if (!this) return 0;
    found = findModule(this, id);
    if (!found) return 0;
    found->delete(found->mod);
    found->mod = 0;
    return 1;
}

int
modrepo_isActive(Modrepo *this, const char *id)
{
    Modrepo *found;

    if (!this) return 0;
    found = findModule(this, id);
    return (found && found->mod) ? 1 : 0;
}

char *
modrepo_getHelp(Modrepo *this, const char *id)
{
    static const char *mainHelpHeader = "* Module `%s':\n";
    static const char *fileHelpHeader = "\n* File options:\n";
    char *helpText;
    size_t helpLen;
    const char *mainHelp, *fileHelp;
    Modrepo *found;

    if (!this) return 0;
    found = findModule(this, id);
    if (!found) return 0;
    mainHelp = found->help ? found->help() : 0;
    fileHelp = found->helpFile ? found->helpFile() : 0;

    if (!mainHelp && !fileHelp) return 0;

    helpLen = strlen(mainHelpHeader) + strlen(id) - 1;

    if (mainHelp) helpLen += strlen(mainHelp);
    if (fileHelp) helpLen += strlen(fileHelpHeader) + strlen(fileHelp);

    helpText = malloc(helpLen);
    sprintf(helpText, mainHelpHeader, id);
    if (mainHelp) strcat(helpText, mainHelp);
    if (fileHelp)
    {
        strcat(helpText, fileHelpHeader);
        strcat(helpText, fileHelp);
    }

    return helpText;
}

void
modrepo_allInitImage(Modrepo *this, Image *image)
{
    Modrepo *current;
    for (current = this; current; current = current->next)
    {
        if (current->mod && current->mod->initImage)
            current->mod->initImage(current->mod, image);
    }
}

void
modrepo_allGlobalOption(Modrepo *this, char opt, const char *arg)
{
    Modrepo *current;
    for (current = this; current; current = current->next)
    {
        if (current->mod && current->mod->globalOption)
            current->mod->globalOption(current->mod, opt, arg);
    }
}

void
modrepo_allFileOption(Modrepo *this, Diskfile *file, char opt, const char *arg)
{
    Modrepo *current;
    for (current = this; current; current = current->next)
    {
        if (current->mod && current->mod->fileOption)
            current->mod->fileOption(current->mod, file, opt, arg);
    }
}

Track *
modrepo_firstGetTrack(Modrepo *this, int track)
{
    Track *t;
    Modrepo *current;

    for (current = this; current; current = current->next)
    {
        if (current->mod && current->mod->getTrack)
        {
            t = current->mod->getTrack(current->mod, track);
            if (t) break;
        }
    }

    return t;
}

void
modrepo_allFileWritten(Modrepo *this,
        Diskfile *file, const BlockPosition *start)
{
    Modrepo *current;
    for (current = this; current; current = current->next)
    {
        if (current->mod && current->mod->fileWritten)
            current->mod->fileWritten(current->mod, file, start);
    }
}

void
modrepo_allStatusChanged(Modrepo *this, const BlockPosition *pos)
{
    Modrepo *current;
    for (current = this; current; current = current->next)
    {
        if (current->mod && current->mod->statusChanged)
            current->mod->statusChanged(current->mod, pos);
    }
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
