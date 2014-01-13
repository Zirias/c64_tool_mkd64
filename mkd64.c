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

struct suggestedOption;
typedef struct suggestedOption SuggestedOption;

struct suggestedOption
{
    SuggestedOption *next;
    IModule *suggestedBy;
    int fileNo;
    char opt;
    char *arg;
    const char *reason;
};

typedef struct
{
    int initialized;
    int currentPass;
    int maxPasses;
    int currentFileNo;
    Image *image;
    Cmdline *cmdline;
    Modrepo *modrepo;
    SuggestedOption *suggestions;
    SuggestedOption *currentSuggestions;
    FILE *d64;
    const char *d64name;
    FILE *map;
    const char *mapname;
} Mkd64;

static Mkd64 mkd64 = {0};

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

SOLOCAL int
mkd64_init(int argc, char **argv)
{
    mkd64.image = image_new();
    mkd64.cmdline = cmdline_new();
    cmdline_parse(mkd64.cmdline, argc, argv);
    mkd64.modrepo = modrepo_new(cmdline_exe(mkd64.cmdline),
            &mkd64, &moduleLoaded);
    mkd64.suggestions = 0;
    mkd64.currentSuggestions = 0;
    mkd64.d64 = 0;
    mkd64.map = 0;
    mkd64.initialized = 1;
    return 1;
}

static void
printVersion(const char *modId)
{
    if (modId)
    {
       char *versionInfo = modrepo_getVersionInfo(mkd64.modrepo, modId);
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
printUsage(void)
{
    const char *exe = cmdline_exe(mkd64.cmdline);
    printVersion(0);
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
printHelp(const char *modId)
{
    fputs("mkd64 " MKD64_VERSION " help\n\n", stderr);

    if (modId)
    {
        char *modHelp = modrepo_getHelp(mkd64.modrepo, modId);
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
processFileSuggestions(Diskfile *file, BlockPosition *pos)
{
    int fileNo = diskfile_fileNo(file);
    SuggestedOption *sopt = mkd64.currentSuggestions;

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
                    diskfile_setInterleave(file, atoi(sopt->arg));
                    break;
            }
            if (sopt->opt != 'w')
            {
                modrepo_allFileOption(mkd64.modrepo, file,
                        sopt->opt, sopt->arg);
            }
        }
        sopt = sopt->next;
    }
}

static int
processFiles(void)
{
    Diskfile *currentFile = 0;
    Diskfile *nextFile;
    BlockPosition pos;
    char opt;
    int handled;
    const char *arg;
    int intarg;

    mkd64.currentFileNo = 0;

    do
    {
        opt = cmdline_opt(mkd64.cmdline);
        arg = cmdline_arg(mkd64.cmdline);
        handled = 0;

        switch (opt)
        {
            case 'f':
                if (currentFile)
                {
                    fprintf(stderr, "Warning: new file started before "
                            "previous file `%s' was written.\n"
                            "         dropping previous file!\n",
                            diskfile_name(currentFile));
                    diskfile_delete(currentFile);
                    currentFile = 0;
                    --mkd64.currentFileNo;
                }
                nextFile = diskfile_new();
                if (arg)
                {
                    if (diskfile_readFromHost(nextFile, arg))
                    {
                        diskfile_setName(nextFile, arg);
                    }
                    else
                    {
                        fprintf(stderr, "Error opening `%s' for reading: %s\n",
                                arg, strerror(errno));
                        diskfile_delete(nextFile);
                        nextFile = 0;
                    }
                }
                if (nextFile)
                {
                    currentFile = nextFile;
                    diskfile_setFileNo(currentFile, ++mkd64.currentFileNo);
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
                    else if (currentFile) diskfile_setInterleave(
                            currentFile, intarg);
                }
                handled = 1;
                break;
            case 'w':
                checkArgAndWarn(opt, arg, 1, 0, 0);
                if (currentFile)
                {
                    processFileSuggestions(currentFile, &pos);
                    if (!diskfile_write(currentFile, mkd64.image, &pos))
                    {
                        fprintf(stderr,
                                "Error: Disk full while writing file #%d.",
                                mkd64.currentFileNo);
                        diskfile_delete(currentFile);
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
            if(modrepo_allFileOption(mkd64.modrepo, currentFile, opt, arg))
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

    } while (cmdline_moveNext(mkd64.cmdline));
    return 1;
}

static void
processSuggestions(void)
{
    SuggestedOption *sopt = mkd64.currentSuggestions;

    while (sopt)
    {
        if (sopt->fileNo == 0)
        {
            modrepo_allGlobalOption(mkd64.modrepo, sopt->opt, sopt->arg);
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
printResult(void)
{
    int tn = 0;
    int free = 0;
    int sn;
    Track *t;
    BlockStatus s;

    puts("mkd64 image creation complete.\n"
            "Blocks on disk (. = free, : = reserved, but free, "
            "# = allocated):");

    while ((t = image_track(mkd64.image, ++tn)))
    {
        printf("%02d: ", tn);
        for (sn = 0; sn < track_numSectors(t); ++sn)
        {
            s = track_blockStatus(t, sn);
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
processSingleOptions(void)
{
    const char *arg, *modid;
    char *argDup;

    if (cmdline_count(mkd64.cmdline) == 1)
    {
        /* handle single options */

        if (cmdline_opt(mkd64.cmdline) == 'V')
        {
            printVersion(cmdline_arg(mkd64.cmdline));
            return 1;
        }

        if (cmdline_opt(mkd64.cmdline) == 'h')
        {
            printHelp(cmdline_arg(mkd64.cmdline));
            return 1;
        }

        if (cmdline_opt(mkd64.cmdline) == 'C')
        {
            arg = cmdline_arg(mkd64.cmdline);
            if (!arg)
            {
                fputs("Error: missing argument to single option -C.\n", stderr);
                return 1;
            }
            argDup = copyString(arg);
            if (!(cmdline_parseFile(mkd64.cmdline, argDup)))
            {
                perror("Error opening or reading commandline file");
                free(argDup);
                return 1;
            }
            free(argDup);
            if (!cmdline_moveNext(mkd64.cmdline))
            {
                fprintf(stderr, "Error: no options found in `%s'.\n", argDup);
                return 1;
            }
        }

        if (cmdline_opt(mkd64.cmdline) == 'M')
        {
            if (cmdline_arg(mkd64.cmdline))
            {
                fputs("Warning: argument to single option -M ignored.\n",
                        stderr);
            }
            fputs("Available modules:\n", stderr);
            modid = 0;
            while ((modid = modrepo_nextAvailableModule(mkd64.modrepo, modid)))
            {
                fprintf(stderr, "  %s\n", modid);
            }
            return 1;
        }
    }

    return 0;
}

static int
processGlobalOptions(void)
{
    char opt;
    const char *arg;
    int intarg, handled;

    do
    {
        opt = cmdline_opt(mkd64.cmdline);
        arg = cmdline_arg(mkd64.cmdline);
        handled = 0;

        switch (opt)
        {
            case 'm':
                handled = 1;
                if (mkd64.currentPass > 1) break;
                if (!checkArgAndWarn(opt, arg, 1, 1, 0)) return 0;
                if (!modrepo_createInstance(mkd64.modrepo, arg))
                {
                    fprintf(stderr, "Error: module `%s' not found.\n", arg);
                    return 0;
                }
                break;
            case 'o':
                handled = 1;
                if (mkd64.currentPass > 1) break;
                if (!checkArgAndWarn(opt, arg, 1, 1, 0)) return 0;
                if (mkd64.d64)
                {
                    fputs("Error: D64 output file specified twice.\n", stderr);
                    return 0;
                }
                mkd64.d64 = fopen(arg, "wb");
                if (!mkd64.d64)
                {
                    perror("Error opening D64 output file");
                    return 0;
                }
                mkd64.d64name = arg;
                break;
            case 'M':
                handled = 1;
                if (mkd64.currentPass > 1) break;
                if (!checkArgAndWarn(opt, arg, 1, 1, 0)) return 0;
                if (mkd64.map)
                {
                    fputs("Error: file map output file specified twice.\n",
                            stderr);
                    return 0;
                }
                mkd64.map = fopen(arg, "w");
                if (!mkd64.map)
                {
                    perror("Error opening file map output file");
                    return 0;
                }
                mkd64.mapname = arg;
                break;
            case 'P':
                handled = 1;
                if (mkd64.currentPass > 1) break;
                if (arg)
                {
                    if (!tryParseInt(arg, &intarg) || intarg < 1)
                    {
                        fprintf(stderr, "Error: invalid maximum passes `%s' "
                                "given.\n", arg);
                        return 0;
                    }
                    mkd64.maxPasses = intarg;
                }
                else
                {
                    mkd64.maxPasses = 5;
                }
                break;
            case 'f':
                return 1;
        }
        if (!(modrepo_allGlobalOption(mkd64.modrepo, opt, arg) || handled))
        {
            fprintf(stderr, "Warning: unrecognized global option -%c ignored.\n"
                   "         Maybe you forgot to load a module?\n", opt);
        }
    } while (cmdline_moveNext(mkd64.cmdline));
    return 1;
}

SOLOCAL int
mkd64_run(void)
{
    if (!mkd64.initialized) return 0;

    if (!cmdline_moveNext(mkd64.cmdline))
    {
        /* no options given */

        printUsage();
        return 0;
    }

    /* started using a "single" option? then it's handled by this call,
     * so just exit */
    if (processSingleOptions()) return 0;

    mkd64.currentPass = 1;
    mkd64.maxPasses = 1;

    /* loop for multiple passes */
    do
    {
        /* first handle global options */
        if (!processGlobalOptions()) goto mkd64_run_error;

        /* the first occurence of '-f' switches to handling files */
        if (cmdline_opt(mkd64.cmdline) == 'f')
        {
            /* if there are suggestions for global options from the previous
             * pass, apply them before handling the files */
            processSuggestions();
            if (!processFiles()) goto mkd64_run_error;
        }

        /* give modules a chance to suggest better options by notifying them
         * that we're done */
        modrepo_allImageComplete(mkd64.modrepo);

        /* cleanup suggestions used in this pass */
        deleteSuggestions(mkd64.currentSuggestions);
        mkd64.currentSuggestions = 0;

        /* if there are new suggestions, check whether we should do another
         * pass */
        if (mkd64.suggestions)
        {
            printSuggestions(mkd64.suggestions);
            if (mkd64.currentPass < mkd64.maxPasses)
            {
                fputs("[Info] rerunning using the above suggestions ...\n",
                        stderr);
                image_reset(mkd64.image);
                modrepo_reloadModules(mkd64.modrepo);
                cmdline_moveNext(mkd64.cmdline);
                mkd64.currentSuggestions = mkd64.suggestions;
                mkd64.suggestions = 0;
                fprintf(stderr, "* Pass #%d\n", ++mkd64.currentPass);
            }
        }
    }
    while (mkd64.currentSuggestions);

    /* write the disk image to output file */
    if (mkd64.d64)
    {
        if (image_dump(mkd64.image, mkd64.d64))
            printf("D64 image written to `%s'.\n", mkd64.d64name);
        else
            perror("Error writing D64 image");
        fclose(mkd64.d64);
        mkd64.d64 = 0;
    }
    else
    {
        fputs("Warning: no output file specified, use -o to save your created "
                ".d64.\n", stderr);
    }

    /* write the file map */
    if (mkd64.map)
    {
        if (filemap_dump(image_filemap(mkd64.image), mkd64.map))
            printf("File map for image written to `%s'.\n", mkd64.mapname);
        else
            perror("Error writing file map");
        fclose(mkd64.map);
        mkd64.map = 0;
    }

    /* show the user what was done */
    printResult();

    return 1;

mkd64_run_error:
    if (mkd64.d64) fclose(mkd64.d64);
    if (mkd64.map) fclose(mkd64.map);
    return 0;
}

SOLOCAL void
mkd64_done(void)
{
    if (!mkd64.initialized) return;
    mkd64.initialized = 0;
    image_delete(mkd64.image);
    cmdline_delete(mkd64.cmdline);
    modrepo_delete(mkd64.modrepo);
    deleteSuggestions(mkd64.suggestions);
    deleteSuggestions(mkd64.currentSuggestions);
}

SOLOCAL Image *
mkd64_image(void)
{
    return mkd64.initialized ? mkd64.image : 0;
}

SOLOCAL Cmdline *
mkd64_cmdline(void)
{
    return mkd64.initialized ? mkd64.cmdline : 0;
}

SOEXPORT Modrepo *
mkd64_modrepo(void)
{
    return mkd64.initialized ? mkd64.modrepo : 0;
}

SOEXPORT void
mkd64_suggestOption(IModule *mod, int fileNo,
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

    if (!mkd64.suggestions) mkd64.suggestions = sopt;
    else
    {
        parent = mkd64.suggestions;
        while (parent->next) parent = parent->next;
        parent->next = sopt;
    }
}

int main(int argc, char **argv)
{
    int exit;

    mkd64_init(argc, argv);
    exit = mkd64_run() ? EXIT_SUCCESS : EXIT_FAILURE;
    mkd64_done();

    return exit;
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
