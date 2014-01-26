#ifdef __cplusplus
#error This is C-Code, it will not correctly compile using a C++ compiler. \
    If you do not have a C-Compiler on your system, try GCC (MinGW for windows)
#endif

#include <mkd64/common.h>
#include <mkd64/imodule.h>
#include <mkd64/util.h>

#include "mkd64.h"
#include "image.h"
#include "diskfile.h"
#include "cmdline.h"
#include "modrepo.h"
#include "buildid.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

typedef struct SuggestedOption SuggestedOption;

struct SuggestedOption
{
    SuggestedOption *next;
    IModule *suggestedBy;
    int fileNo;
    char opt;
    char *arg;
    const char *reason;
};

struct Mkd64
{
    int initialized;
    int currentPass;
    int maxPasses;
    int currentFileNo;
    Image *image;
    Cmdline *cmdline;
    ModRepo *modrepo;
    SuggestedOption *suggestions;
    SuggestedOption *currentSuggestions;
    FILE *d64;
    const char *d64name;
    FILE *map;
    const char *mapname;
};

static Mkd64 *instance = 0;

static void
moduleLoaded(void *owner, IModule *mod)
{
    Mkd64 *self = owner;
    if (mod->initImage) mod->initImage(mod, self->image);
}

static void
deleteSuggestions(SuggestedOption *suggestions)
{
    SuggestedOption *tmp = suggestions;
    while (tmp)
    {
        suggestions = suggestions->next;
        free(tmp->arg);
        free(tmp);
        tmp = suggestions;
    }
}

SOLOCAL Mkd64 *
Mkd64_init(Mkd64 *self, int argc, char **argv)
{
    if (!instance)
    {
        instance = self;
        memset(self, 0, sizeof(Mkd64));

        self->cmdline = OBJNEW(Cmdline);
        Cmdline_parse(self->cmdline, argc, argv);
        self->image = OBJNEW(Image);

        self->initialized = 1;

        /* create ModRepo after Mkd64 is initialized, because it may need
         * util functions requiring an Mkd64 instance */
        self->modrepo = OBJNEW2(ModRepo, self, &moduleLoaded);
    }
    else
    {
        self->initialized = 0;
    }
    return self;
}

static void
printVersion(Mkd64 *self, const char *modId)
{
    if (modId)
    {
       char *versionInfo = ModRepo_getVersionInfo(self->modrepo, modId);
       if (versionInfo)
       {
           fputs(versionInfo, stderr);
           free(versionInfo);
       }
       else
       {
           fprintf(stderr, "Module `%s' not found.\n", modId);
       }
    }
    else
    {
        fputs("mkd64 " MKD64_VERSION "\n"
                "a modular tool for creating D64 disk images.\n"
                "Felix Palmen (Zirias) -- <felix@palmen-it.de>\n\n"
                BUILDID_ALL "\n", stderr);
    }
}

static void
printUsage(Mkd64 *self)
{
    const char *exe = Cmdline_exe(self->cmdline);
    printVersion(self, 0);
    fprintf(stderr, "\nUSAGE: %s -h [MODULE]\n"
            "       %s -V [MODULE]\n"
            "       %s -C OPTFILE\n"
            "       %s -M\n"
            "       %s OPTION [ARGUMENT] [OPTION [ARGUMENT]...]\n"
            "           [FILEOPTION [ARGUMENT]...]\n\n"
            "type `%s -h' for help on available options and fileoptions.\n",
            exe, exe, exe, exe, exe, exe);
}

static void
printHelp(Mkd64 *self, const char *modId)
{
    fputs("mkd64 " MKD64_VERSION " help\n\n", stderr);

    if (modId)
    {
        char *modHelp = ModRepo_getHelp(self->modrepo, modId);
        if (modHelp)
        {
            fputs(modHelp, stderr);
            free(modHelp);
        }
        else
        {
            fprintf(stderr, "Module `%s' not found.\n", modId);
        }
    }
    else
    {
        fputs(
"mkd64 supports four types of options. Single options trigger some immediate\n"
"action, see below. Global options affect the whole disk image generation,\n"
"module options are passed just to the last loaded module and file options \n"
"control single files written to the image.\n"
"Global and module options must come before file options on the command line."
"\n\n"
"Modules can provide their own global and file options, check their help\n"
"messages (-h MODULE) for reference.\n\n"
"SINGLE options (must be the only option to mkd64):\n"
"  -h [MODULE]    Show this help message or, if given, the help message for\n"
"                 the module {MODULE}, and exit.\n"
"  -V [MODULE]    Show version info and exit. If {MODULE} is given, version\n"
"                 info for that module is shown instead.\n"
"  -C OPTFILE     Read options from file {OPTFILE} instead of the command\n"
"                 line. The file has the same format as the normal command\n"
"                 line and the following rules:\n"
"                 - Strings containing whitespace are escaped using quotes\n"
"                   or doublequotes (' or \")\n"
"                 - The backslash (\\) has no special meaning at all\n"
"                 - Newlines are just normal whitspace and thus ignored\n"
"  -M             Display all available modules and exit.\n\n"
"GLOBAL options:\n"
"  -m MODULE      Activate module {MODULE}. Modules are searched for in the\n"
"                 directory of the mkd64 executable for a portable build of\n"
"                 mkd64, or in the dedicated module directory (typically\n"
"                 /usr/lib/mkd64) for an installable build. Any options\n"
"                 following -m are treated as module options to this module,\n"
"                 as long as there is no other -m option or a -g option to\n"
"                 get back to global scope or a -f option to switch to file\n"
"                 scope.\n"
"  -o D64FILE     Write generated disk image to {D64FILE}. This option must\n"
"                 be given to actually write something.\n"
"  -M MAPFILE     Write file map of the generated disk image to MAPFILE. The\n"
"                 map file format is one line per file on disk:\n"
"                 [startTrack];[startSector];[filename]\n"
"  -P [MAXPASSES] Allow up to {MAXPASSES} passes, automatically applying\n"
"                 options suggested by modules. The default is only one pass\n"
"                 if this option is not given or up to 5 passes if it is\n"
"                 given without an argument.\n\n"
"MODULE options:\n"
"  -g             Go back to global scope after loading a module.\n"
"                 Please see the module documentation or help text\n"
"                 (-h MODULE) for options available with specific modules.\n\n"
"FILE options:\n"
"  -f [FILENAME]  Start a new file. {FILENAME} is the name on your PC. It\n"
"                 can be omitted for special emtpy files.\n"
"  -t TRACK       Set fixed start track for current file.\n"
"  -s SECTOR      Set fixed start sector for current file.\n"
"  -i INTERLEAVE  Set sector interleave for current file.\n"
"  -w             Write current file to disk image.\n\n"
"Note that filesystem elements (like the original cbmdos directory and BAM)\n"
"are implemented by modules. They can provide a sensible default value for\n"
"sector interleave. A default allocation strategy is built in and determines\n"
"start track and sector automatically if not given, modules can install their\n"
"own strategy.\n\n", stderr);
    }
}

static void
processFileSuggestions(Mkd64 *self, DiskFile *file, BlockPosition *pos)
{
    int fileNo = DiskFile_fileNo(file);
    SuggestedOption *sopt = self->currentSuggestions;

    while (sopt)
    {
        if (sopt->fileNo == fileNo)
        {
            switch (sopt->opt)
            {
                case 't':
                    pos->track = atoi(sopt->arg);
                    break;
                case 's':
                    pos->sector = atoi(sopt->arg);
                    break;
                case 'i':
                    DiskFile_setInterleave(file, atoi(sopt->arg));
                    break;
            }
            if (sopt->opt != 'w')
            {
                ModRepo_allFileOption(self->modrepo, file,
                        sopt->opt, sopt->arg);
            }
        }
        sopt = sopt->next;
    }
}

static int
processFiles(Mkd64 *self)
{
    DiskFile *currentFile = 0;
    DiskFile *nextFile;
    BlockPosition pos;
    char opt;
    int handled;
    const char *arg;
    int intarg;

    self->currentFileNo = 0;

    do
    {
        opt = Cmdline_opt(self->cmdline);
        arg = Cmdline_arg(self->cmdline);
        handled = 0;

        switch (opt)
        {
            case 'f':
                if (currentFile)
                {
                    fprintf(stderr, "Warning: new file started before "
                            "previous file `%s' was written.\n"
                            "         dropping previous file!\n",
                            DiskFile_name(currentFile));
                    OBJDEL(DiskFile, currentFile);
                    currentFile = 0;
                    --self->currentFileNo;
                }
                nextFile = OBJNEW(DiskFile);
                if (arg)
                {
                    if (DiskFile_readFromHost(nextFile, arg))
                    {
                        DiskFile_setName(nextFile, arg);
                    }
                    else
                    {
                        fprintf(stderr, "Error opening `%s' for reading: %s\n",
                                arg, strerror(errno));
                        OBJDEL(DiskFile, nextFile);
                        nextFile = 0;
                    }
                }
                if (nextFile)
                {
                    currentFile = nextFile;
                    DiskFile_setFileNo(currentFile, ++self->currentFileNo);
                    pos.track = 0;
                    pos.sector = 0;
                }
                handled = 1;
                break;
            case 't':
                if (checkArgAndWarn(opt, arg, 1, 1, 0))
                {
                    if (!tryParseInt(arg, &intarg) || intarg < 1)
                    {
                        fprintf(stderr, "Warning: invalid track number `%s' "
                                "ignored.\n", arg);
                    }
                    else if (currentFile) pos.track = intarg;
                }
                handled = 1;
                break;
            case 's':
                if (checkArgAndWarn(opt, arg, 1, 1, 0))
                {
                    if (!tryParseInt(arg, &intarg) || intarg < 0)
                    {
                        fprintf(stderr, "Warning: invalid sector number `%s' "
                                "ignored.\n", arg);
                    }
                    else if (currentFile) pos.sector = intarg;
                }
                handled = 1;
                break;
            case 'i':
                if (checkArgAndWarn(opt, arg, 1, 1, 0))
                {
                    if (!tryParseInt(arg, &intarg) || intarg < 1)
                    {
                        fprintf(stderr, "Warning: invalid interleave number "
                                "`%s' ignored.\n", arg);
                    }
                    else if (currentFile) DiskFile_setInterleave(
                            currentFile, intarg);
                }
                handled = 1;
                break;
            case 'w':
                checkArgAndWarn(opt, arg, 1, 0, 0);
                if (currentFile)
                {
                    processFileSuggestions(self, currentFile, &pos);
                    if (!DiskFile_write(currentFile, self->image, &pos))
                    {
                        fprintf(stderr,
                                "Error: Disk full while writing file #%d.",
                                self->currentFileNo);
                        OBJDEL(DiskFile, currentFile);
                        return 0;
                    }
                    currentFile = 0;
                }
                else
                {
                    fputs("Warning: option -w given without starting a file "
                            "before. Option ignored.\n", stderr);
                }
                handled = 1;
                break;
        }
        if (currentFile)
        {
            if(ModRepo_allFileOption(self->modrepo, currentFile, opt, arg))
                handled = 1;
        }
        else if (opt != 'w')
        {
            fprintf(stderr, "Warning: file option `-%c' given without "
                    "starting a file ignored.\n", opt);
        }
        if (!handled)
        {
            fprintf(stderr, "Warning: unrecognized file option -%c ignored.\n"
                   "         Maybe you forgot to load a module?\n", opt);
        }

    } while (Cmdline_moveNext(self->cmdline));
    return 1;
}

static void
processSuggestions(Mkd64 *self)
{
    SuggestedOption *sopt = self->currentSuggestions;

    while (sopt)
    {
        if (sopt->fileNo == 0)
        {
            sopt->suggestedBy->option(sopt->suggestedBy, sopt->opt, sopt->arg);
        }
        sopt = sopt->next;
    }
}

static void
printSuggestions(SuggestedOption *suggestions)
{
    static const char *empty = "";

    while (suggestions)
    {
        if (suggestions->fileNo > 0)
        {
            fprintf(stderr,
                    "[Hint] %s suggests option -%c%s for file #%d: %s\n",
                    suggestions->suggestedBy->id(), suggestions->opt,
                    suggestions->arg ? suggestions->arg : empty,
                    suggestions->fileNo, suggestions->reason);
        }
        else
        {
            fprintf(stderr, "[Hint] %s suggests option -%c%s: %s\n",
                    suggestions->suggestedBy->id(), suggestions->opt,
                    suggestions->arg ? suggestions->arg : empty,
                    suggestions->reason);
        }
        suggestions = suggestions->next;
    }
}

static void
printResult(Mkd64 *self)
{
    int tn = 0;
    int free = 0;
    unsigned int sn;
    Track *t;
    BlockStatus s;

    puts("mkd64 image creation complete.\n"
            "Blocks on disk (. = free, : = reserved, but free, "
            "# = allocated):");

    while ((t = Image_track(self->image, ++tn)))
    {
        printf("%02d: ", tn);
        for (sn = 0; sn < Track_numSectors(t); ++sn)
        {
            s = Track_blockStatus(t, sn);
            if (s & BS_ALLOCATED) putc('#', stdout);
            else if (s & BS_RESERVED) putc(':', stdout);
            else
            {
                ++free;
                putc('.', stdout);
            }
        }
        putc('\n', stdout);
    }

    printf("%d blocks free.\n", free);
}

static int
processSingleOptions(Mkd64 *self)
{
    const char *arg, *modid;
    char *argDup;

    if (Cmdline_count(self->cmdline) == 1)
    {
        /* handle single options */

        if (Cmdline_opt(self->cmdline) == 'V')
        {
            printVersion(self, Cmdline_arg(self->cmdline));
            return 1;
        }

        if (Cmdline_opt(self->cmdline) == 'h')
        {
            printHelp(self, Cmdline_arg(self->cmdline));
            return 1;
        }

        if (Cmdline_opt(self->cmdline) == 'C')
        {
            arg = Cmdline_arg(self->cmdline);
            if (!arg)
            {
                fputs("Error: missing argument to single option -C.\n", stderr);
                return 1;
            }
            argDup = copyString(arg);
            if (!(Cmdline_parseFile(self->cmdline, argDup)))
            {
                perror("Error opening or reading commandline file");
                free(argDup);
                return 1;
            }
            free(argDup);
            if (!Cmdline_moveNext(self->cmdline))
            {
                fprintf(stderr, "Error: no options found in `%s'.\n", argDup);
                return 1;
            }
        }

        if (Cmdline_opt(self->cmdline) == 'M')
        {
            if (Cmdline_arg(self->cmdline))
            {
                fputs("Warning: argument to single option -M ignored.\n",
                        stderr);
            }
            fputs("Available modules:\n", stderr);
            modid = 0;
            while ((modid = ModRepo_nextAvailableModule(self->modrepo, modid)))
            {
                fprintf(stderr, "  %s\n", modid);
            }
            return 1;
        }
    }

    return 0;
}

static int
processModule(Mkd64 *self, ModInstIterator *iter)
{
    IModule *mod;
    char opt;
    const char *arg = Cmdline_arg(self->cmdline);

    if (iter)
    {
        mod = ModInstIterator_current(iter);
    }
    else
    {
        mod = ModRepo_createInstance(self->modrepo, arg);
    }

    if (!mod)
    {
        fprintf(stderr, "Error: module `%s' not found.\n", arg);
        return -1;
    }

    while (Cmdline_moveNext(self->cmdline))
    {
        opt = Cmdline_opt(self->cmdline);
        arg = Cmdline_arg(self->cmdline);

        if (opt == 'g' || opt == 'm' || opt == 'f') return 0;

        if (!mod->option || !mod->option(mod, opt, arg))
        {
            fprintf(stderr, "Warning: module `%s' did not understand option "
                    "-%c.\n", mod->id(), opt);
        }
    }
    return 1;
}

static int
processGlobalOptions(Mkd64 *self)
{
    ModInstIterator *iter = 0;
    char opt;
    const char *arg;
    int intarg, handled;

    do
    {
processGlobalOptions_restart:

        opt = Cmdline_opt(self->cmdline);
        arg = Cmdline_arg(self->cmdline);
        handled = 0;

        switch (opt)
        {
            case 'm':
                if (!checkArgAndWarn(opt, arg, 1, 1, 0))
                    goto processGlobalOptions_error;
                if (self->currentPass > 1)
                {
                    if (!iter) iter = ModRepo_createIterator(self->modrepo);
                    ModInstIterator_moveNext(iter);
                }
                handled = processModule(self, iter);
                if (handled < 0) goto processGlobalOptions_error;
                if (handled > 0) goto processGlobalOptions_success;
                goto processGlobalOptions_restart;
            case 'g':
                /* ignored, used to come back here from processModule() */
                handled = 1;
                break;
            case 'o':
                handled = 1;
                if (self->currentPass > 1) break;
                if (!checkArgAndWarn(opt, arg, 1, 1, 0))
                    goto processGlobalOptions_error;
                if (self->d64)
                {
                    fputs("Error: D64 output file specified twice.\n", stderr);
                    goto processGlobalOptions_error;
                }
                self->d64 = fopen(arg, "wb");
                if (!self->d64)
                {
                    perror("Error opening D64 output file");
                    goto processGlobalOptions_error;
                }
                self->d64name = arg;
                break;
            case 'M':
                handled = 1;
                if (self->currentPass > 1) break;
                if (!checkArgAndWarn(opt, arg, 1, 1, 0))
                    goto processGlobalOptions_error;
                if (self->map)
                {
                    fputs("Error: file map output file specified twice.\n",
                            stderr);
                    goto processGlobalOptions_error;
                }
                self->map = fopen(arg, "w");
                if (!self->map)
                {
                    perror("Error opening file map output file");
                    goto processGlobalOptions_error;
                }
                self->mapname = arg;
                break;
            case 'P':
                handled = 1;
                if (self->currentPass > 1) break;
                if (arg)
                {
                    if (!tryParseInt(arg, &intarg) || intarg < 1)
                    {
                        fprintf(stderr, "Error: invalid maximum passes `%s' "
                                "given.\n", arg);
                        goto processGlobalOptions_error;
                    }
                    self->maxPasses = intarg;
                }
                else
                {
                    self->maxPasses = 5;
                }
                break;
            case 'f':
                goto processGlobalOptions_success;
        }
        if (!(ModRepo_allGlobalOption(self->modrepo, opt, arg) || handled))
        {
            fprintf(stderr, "Warning: unrecognized global option -%c ignored.\n"
                   "         Maybe you forgot to load a module?\n", opt);
        }
    } while (Cmdline_moveNext(self->cmdline));

processGlobalOptions_success:
    if (iter) ModInstIterator_free(iter);
    return 1;

processGlobalOptions_error:
    if (iter) ModInstIterator_free(iter);
    return 0;
}

static void
cleanupRunObjects(Mkd64 *self)
{
    if (self->d64) fclose(self->d64);
    self->d64 = 0;
    if (self->map) fclose(self->map);
    self->map = 0;
    deleteSuggestions(self->suggestions);
    deleteSuggestions(self->currentSuggestions);
}

SOLOCAL int
Mkd64_run(Mkd64 *self)
{
    if (!self->initialized) return 0;

    if (!Cmdline_moveNext(self->cmdline))
    {
        /* no options given */

        printUsage(self);
        return 0;
    }

    /* started using a "single" option? then it's handled by self call,
     * so just exit */
    if (processSingleOptions(self)) return 0;

    self->currentPass = 1;
    self->maxPasses = 1;

    /* loop for multiple passes */
    do
    {
        /* first handle global options */
        if (!processGlobalOptions(self)) goto Mkd64_run_error;

        /* the first occurence of '-f' switches to handling files */
        if (Cmdline_opt(self->cmdline) == 'f')
        {
            /* if there are suggestions for private options from the previous
             * pass, apply them before handling the files */
            processSuggestions(self);
            if (!processFiles(self)) goto Mkd64_run_error;
        }

        /* give modules a chance to suggest better options by notifying them
         * that we're done */
        ModRepo_allImageComplete(self->modrepo);

        /* cleanup suggestions used in self pass */
        deleteSuggestions(self->currentSuggestions);
        self->currentSuggestions = 0;

        /* if there are new suggestions, check whether we should do another
         * pass */
        if (self->suggestions)
        {
            printSuggestions(self->suggestions);
            if (self->currentPass < self->maxPasses)
            {
                fputs("[Info] rerunning using the above suggestions ...\n",
                        stderr);
                Image_done(self->image);
                Image_init(self->image);
                ModRepo_allInitImage(self->modrepo, self->image);
                Cmdline_moveNext(self->cmdline);
                self->currentSuggestions = self->suggestions;
                self->suggestions = 0;
                fprintf(stderr, "* Pass #%d\n", ++self->currentPass);
            }
        }
    }
    while (self->currentSuggestions);

    /* write the disk image to output file */
    if (self->d64)
    {
        if (Image_dump(self->image, self->d64))
            printf("D64 image written to `%s'.\n", self->d64name);
        else
            perror("Error writing D64 image");
        fclose(self->d64);
        self->d64 = 0;
    }
    else
    {
        fputs("Warning: no output file specified, use -o to save your created "
                ".d64.\n", stderr);
    }

    /* write the file map */
    if (self->map)
    {
        if (FileMap_dump(Image_fileMap(self->image), self->map))
            printf("File map for image written to `%s'.\n", self->mapname);
        else
            perror("Error writing file map");
        fclose(self->map);
        self->map = 0;
    }

    /* show the user what was done */
    printResult(self);

    cleanupRunObjects(self);
    return 1;

Mkd64_run_error:
    cleanupRunObjects(self);
    return 0;
}

SOLOCAL void
Mkd64_done(Mkd64 *self)
{
    if (!self->initialized) return;
    OBJDEL(Image, self->image);
    self->image = 0;
    OBJDEL(ModRepo, self->modrepo);
    self->modrepo = 0;
    OBJDEL(Cmdline, self->cmdline);
    self->cmdline = 0;
    instance = 0;
    self->initialized = 0;
}

SOLOCAL Image *
Mkd64_image(Mkd64 *self)
{
    return self->initialized ? self->image : 0;
}

SOLOCAL Cmdline *
Mkd64_cmdline(Mkd64 *self)
{
    return self->initialized ? self->cmdline : 0;
}

SOEXPORT ModRepo *
Mkd64_modRepo(Mkd64 *self)
{
    return self->initialized ? self->modrepo : 0;
}

SOEXPORT void
Mkd64_suggestOption(Mkd64 *self, IModule *mod, int fileNo,
        char opt, const char *arg, const char *reason)
{
    SuggestedOption *sopt = mkd64Alloc(sizeof(SuggestedOption));
    SuggestedOption *parent;

    sopt->next = 0;
    sopt->suggestedBy = mod;
    sopt->fileNo = fileNo;
    sopt->opt = opt;
    sopt->arg = arg ? copyString(arg) : 0;
    sopt->reason = reason;

    if (!self->suggestions) self->suggestions = sopt;
    else
    {
        parent = self->suggestions;
        while (parent->next) parent = parent->next;
        parent->next = sopt;
    }
}

SOEXPORT Mkd64 *
Mkd64_instance(void)
{
    return instance;
}

int main(int argc, char **argv)
{
    static int exit;
    static Mkd64 mkd64;

    Mkd64_init(&mkd64, argc, argv);
    exit = Mkd64_run(&mkd64) ? EXIT_SUCCESS : EXIT_FAILURE;
    Mkd64_done(&mkd64);

    return exit;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
