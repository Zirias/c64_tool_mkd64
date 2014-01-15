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
    Mkd64 *this = owner;
    if (mod->initImage) mod->initImage(mod, this->image);
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
Mkd64_init(Mkd64 *this, int argc, char **argv)
{
    if (!instance)
    {
        instance = this;
        this->image = OBJNEW(Image);
        this->cmdline = OBJNEW(Cmdline);
        Cmdline_parse(this->cmdline, argc, argv);
        this->modrepo = OBJNEW3(ModRepo, Cmdline_exe(this->cmdline),
                this, &moduleLoaded);
        this->suggestions = 0;
        this->currentSuggestions = 0;
        this->d64 = 0;
        this->map = 0;
        this->initialized = 1;
    }
    else
    {
        this->initialized = 0;
    }
    return this;
}

static void
printVersion(Mkd64 *this, const char *modId)
{
    if (modId)
    {
       char *versionInfo = ModRepo_getVersionInfo(this->modrepo, modId);
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
printUsage(Mkd64 *this)
{
    const char *exe = Cmdline_exe(this->cmdline);
    printVersion(this, 0);
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
printHelp(Mkd64 *this, const char *modId)
{
    fputs("mkd64 " MKD64_VERSION " help\n\n", stderr);

    if (modId)
    {
        char *modHelp = ModRepo_getHelp(this->modrepo, modId);
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
"mkd64 supports three types of options. Single options trigger some immediate\n"
"action, see below. Global options affect the whole disk image generation,\n"
"and file options control single files written to the image.\n"
"Global options must come before all file options on the command line.\n\n"
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
"                 directory of the mkd64 executable.\n"
"  -o D64FILE     Write generated disk image to {D64FILE}. This option must\n"
"                 be given to actually write something.\n"
"  -M MAPFILE     Write file map of the generated disk image to MAPFILE. The\n"
"                 map file format is one line per file on disk:\n"
"                 [startTrack];[startSector];[filename]\n"
"  -P [MAXPASSES] Allow up to {MAXPASSES} passes, automatically applying\n"
"                 options suggested by modules. The default is only one pass\n"
"                 if this option is not given or up to 5 passes if it is\n"
"                 given without an argument.\n\n"
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
processFileSuggestions(Mkd64 *this, DiskFile *file, BlockPosition *pos)
{
    int fileNo = DiskFile_fileNo(file);
    SuggestedOption *sopt = this->currentSuggestions;

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
                ModRepo_allFileOption(this->modrepo, file,
                        sopt->opt, sopt->arg);
            }
        }
        sopt = sopt->next;
    }
}

static int
processFiles(Mkd64 *this)
{
    DiskFile *currentFile = 0;
    DiskFile *nextFile;
    BlockPosition pos;
    char opt;
    int handled;
    const char *arg;
    int intarg;

    this->currentFileNo = 0;

    do
    {
        opt = Cmdline_opt(this->cmdline);
        arg = Cmdline_arg(this->cmdline);
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
                    --this->currentFileNo;
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
                    DiskFile_setFileNo(currentFile, ++this->currentFileNo);
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
                    processFileSuggestions(this, currentFile, &pos);
                    if (!DiskFile_write(currentFile, this->image, &pos))
                    {
                        fprintf(stderr,
                                "Error: Disk full while writing file #%d.",
                                this->currentFileNo);
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
            if(ModRepo_allFileOption(this->modrepo, currentFile, opt, arg))
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

    } while (Cmdline_moveNext(this->cmdline));
    return 1;
}

static void
processSuggestions(Mkd64 *this)
{
    SuggestedOption *sopt = this->currentSuggestions;

    while (sopt)
    {
        if (sopt->fileNo == 0)
        {
            ModRepo_allGlobalOption(this->modrepo, sopt->opt, sopt->arg);
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
            fprintf(stderr, "[Hint] %s suggests global option -%c%s: %s\n",
                    suggestions->suggestedBy->id(), suggestions->opt,
                    suggestions->arg ? suggestions->arg : empty,
                    suggestions->reason);
        }
        suggestions = suggestions->next;
    }
}

static void
printResult(Mkd64 *this)
{
    int tn = 0;
    int free = 0;
    int sn;
    Track *t;
    BlockStatus s;

    puts("mkd64 image creation complete.\n"
            "Blocks on disk (. = free, : = reserved, but free, "
            "# = allocated):");

    while ((t = Image_track(this->image, ++tn)))
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
processSingleOptions(Mkd64 *this)
{
    const char *arg, *modid;
    char *argDup;

    if (Cmdline_count(this->cmdline) == 1)
    {
        /* handle single options */

        if (Cmdline_opt(this->cmdline) == 'V')
        {
            printVersion(this, Cmdline_arg(this->cmdline));
            return 1;
        }

        if (Cmdline_opt(this->cmdline) == 'h')
        {
            printHelp(this, Cmdline_arg(this->cmdline));
            return 1;
        }

        if (Cmdline_opt(this->cmdline) == 'C')
        {
            arg = Cmdline_arg(this->cmdline);
            if (!arg)
            {
                fputs("Error: missing argument to single option -C.\n", stderr);
                return 1;
            }
            argDup = copyString(arg);
            if (!(Cmdline_parseFile(this->cmdline, argDup)))
            {
                perror("Error opening or reading commandline file");
                free(argDup);
                return 1;
            }
            free(argDup);
            if (!Cmdline_moveNext(this->cmdline))
            {
                fprintf(stderr, "Error: no options found in `%s'.\n", argDup);
                return 1;
            }
        }

        if (Cmdline_opt(this->cmdline) == 'M')
        {
            if (Cmdline_arg(this->cmdline))
            {
                fputs("Warning: argument to single option -M ignored.\n",
                        stderr);
            }
            fputs("Available modules:\n", stderr);
            modid = 0;
            while ((modid = ModRepo_nextAvailableModule(this->modrepo, modid)))
            {
                fprintf(stderr, "  %s\n", modid);
            }
            return 1;
        }
    }

    return 0;
}

static int
processGlobalOptions(Mkd64 *this)
{
    char opt;
    const char *arg;
    int intarg, handled;

    do
    {
        opt = Cmdline_opt(this->cmdline);
        arg = Cmdline_arg(this->cmdline);
        handled = 0;

        switch (opt)
        {
            case 'm':
                handled = 1;
                if (this->currentPass > 1) break;
                if (!checkArgAndWarn(opt, arg, 1, 1, 0)) return 0;
                if (!ModRepo_createInstance(this->modrepo, arg))
                {
                    fprintf(stderr, "Error: module `%s' not found.\n", arg);
                    return 0;
                }
                break;
            case 'o':
                handled = 1;
                if (this->currentPass > 1) break;
                if (!checkArgAndWarn(opt, arg, 1, 1, 0)) return 0;
                if (this->d64)
                {
                    fputs("Error: D64 output file specified twice.\n", stderr);
                    return 0;
                }
                this->d64 = fopen(arg, "wb");
                if (!this->d64)
                {
                    perror("Error opening D64 output file");
                    return 0;
                }
                this->d64name = arg;
                break;
            case 'M':
                handled = 1;
                if (this->currentPass > 1) break;
                if (!checkArgAndWarn(opt, arg, 1, 1, 0)) return 0;
                if (this->map)
                {
                    fputs("Error: file map output file specified twice.\n",
                            stderr);
                    return 0;
                }
                this->map = fopen(arg, "w");
                if (!this->map)
                {
                    perror("Error opening file map output file");
                    return 0;
                }
                this->mapname = arg;
                break;
            case 'P':
                handled = 1;
                if (this->currentPass > 1) break;
                if (arg)
                {
                    if (!tryParseInt(arg, &intarg) || intarg < 1)
                    {
                        fprintf(stderr, "Error: invalid maximum passes `%s' "
                                "given.\n", arg);
                        return 0;
                    }
                    this->maxPasses = intarg;
                }
                else
                {
                    this->maxPasses = 5;
                }
                break;
            case 'f':
                return 1;
        }
        if (!(ModRepo_allGlobalOption(this->modrepo, opt, arg) || handled))
        {
            fprintf(stderr, "Warning: unrecognized global option -%c ignored.\n"
                   "         Maybe you forgot to load a module?\n", opt);
        }
    } while (Cmdline_moveNext(this->cmdline));
    return 1;
}

SOLOCAL int
Mkd64_run(Mkd64 *this)
{
    if (!this->initialized) return 0;

    if (!Cmdline_moveNext(this->cmdline))
    {
        /* no options given */

        printUsage(this);
        return 0;
    }

    /* started using a "single" option? then it's handled by this call,
     * so just exit */
    if (processSingleOptions(this)) return 0;

    this->currentPass = 1;
    this->maxPasses = 1;

    /* loop for multiple passes */
    do
    {
        /* first handle global options */
        if (!processGlobalOptions(this)) goto Mkd64_run_error;

        /* the first occurence of '-f' switches to handling files */
        if (Cmdline_opt(this->cmdline) == 'f')
        {
            /* if there are suggestions for global options from the previous
             * pass, apply them before handling the files */
            processSuggestions(this);
            if (!processFiles(this)) goto Mkd64_run_error;
        }

        /* give modules a chance to suggest better options by notifying them
         * that we're done */
        ModRepo_allImageComplete(this->modrepo);

        /* cleanup suggestions used in this pass */
        deleteSuggestions(this->currentSuggestions);
        this->currentSuggestions = 0;

        /* if there are new suggestions, check whether we should do another
         * pass */
        if (this->suggestions)
        {
            printSuggestions(this->suggestions);
            if (this->currentPass < this->maxPasses)
            {
                fputs("[Info] rerunning using the above suggestions ...\n",
                        stderr);
                Image_done(this->image);
                Image_init(this->image);
                ModRepo_reloadModules(this->modrepo);
                Cmdline_moveNext(this->cmdline);
                this->currentSuggestions = this->suggestions;
                this->suggestions = 0;
                fprintf(stderr, "* Pass #%d\n", ++this->currentPass);
            }
        }
    }
    while (this->currentSuggestions);

    /* write the disk image to output file */
    if (this->d64)
    {
        if (Image_dump(this->image, this->d64))
            printf("D64 image written to `%s'.\n", this->d64name);
        else
            perror("Error writing D64 image");
        fclose(this->d64);
        this->d64 = 0;
    }
    else
    {
        fputs("Warning: no output file specified, use -o to save your created "
                ".d64.\n", stderr);
    }

    /* write the file map */
    if (this->map)
    {
        if (FileMap_dump(Image_fileMap(this->image), this->map))
            printf("File map for image written to `%s'.\n", this->mapname);
        else
            perror("Error writing file map");
        fclose(this->map);
        this->map = 0;
    }

    /* show the user what was done */
    printResult(this);

    return 1;

Mkd64_run_error:
    if (this->d64) fclose(this->d64);
    if (this->map) fclose(this->map);
    return 0;
}

SOLOCAL void
Mkd64_done(Mkd64 *this)
{
    if (!this->initialized) return;
    this->initialized = 0;
    OBJDEL(Image, this->image);
    OBJDEL(Cmdline, this->cmdline);
    OBJDEL(ModRepo, this->modrepo);
    deleteSuggestions(this->suggestions);
    deleteSuggestions(this->currentSuggestions);
}

SOLOCAL Image *
Mkd64_image(Mkd64 *this)
{
    return this->initialized ? this->image : 0;
}

SOLOCAL Cmdline *
Mkd64_cmdline(Mkd64 *this)
{
    return this->initialized ? this->cmdline : 0;
}

SOEXPORT ModRepo *
Mkd64_modRepo(Mkd64 *this)
{
    return this->initialized ? this->modrepo : 0;
}

SOEXPORT void
Mkd64_suggestOption(Mkd64 *this, IModule *mod, int fileNo,
        char opt, const char *arg, const char *reason)
{
    SuggestedOption *sopt = malloc(sizeof(SuggestedOption));
    SuggestedOption *parent;

    sopt->next = 0;
    sopt->suggestedBy = mod;
    sopt->fileNo = fileNo;
    sopt->opt = opt;
    sopt->arg = arg ? copyString(arg) : 0;
    sopt->reason = reason;

    if (!this->suggestions) this->suggestions = sopt;
    else
    {
        parent = this->suggestions;
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
