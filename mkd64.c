
#include "mkd64.h"
#include "image.h"
#include "cmdline.h"
#include "modrepo.h"
#include "imodule.h"

#include <stdio.h>
#include <errno.h>

typedef struct
{
    int initialized;
    Image *image;
    Cmdline *cmdline;
    Modrepo *modrepo;
    FILE *d64;
    FILE *map;
} Mkd64;

static Mkd64 mkd64 = {0};

int
mkd64_init(int argc, char **argv)
{
    mkd64.image = image_new();
    mkd64.cmdline = cmdline_new();
    cmdline_parse(mkd64.cmdline, argc, argv);
    mkd64.modrepo = modrepo_new(cmdline_exe(mkd64.cmdline));
    mkd64.d64 = 0;
    mkd64.map = 0;
    mkd64.initialized = 1;
    return 1;
}

static void
printVersion(void)
{
    fputs("mkd64 " MKD64_VERSION "\n"
            "a modular tool for creating D64 disk images.\n"
            "Felix Palmen (Zirias) -- <felix@palmen-it.de>\n", stderr);
}

static void
printUsage(void)
{
    const char *exe = cmdline_exe(mkd64.cmdline);
    printVersion();
    fprintf(stderr, "\nUSAGE: %s -h [MODULE]\n"
            "       %s -V\n"
            "       %s OPTION [ARGUMENT] [OPTION [ARGUMENT]...]\n"
            "           [FILEOPTION [ARGUMENT]...]\n\n"
            "type `%s -h' for help on available options and fileoptions.\n",
            exe, exe);
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
"mkd64 supports two types of options. Global options affect the whole disk\n"
"image generation, file options control single files written to the image.\n"
"Global options must come before all file options on the command line.\n\n"
"Modules can provide their own global and file options, check their help\n"
"messages (-? MODULE) for reference.\n\n"
"GLOBAL options:\n"
"  -h [MODULE]    Show this help message or, if given, the help message for\n"
"                 the module {MODULE}, and exit (must be the first option)\n"
"  -V             Show version info and exit (must be the first option)\n\n"
"  -m MODULE      Activate module {MODULE}. Modules are searched for in the\n"
"                 directory of the mkd64 executable.\n"
"  -o D64FILE     Write generated disk image to {D64FILE}. This option must\n"
"                 be given to actually write something.\n"
"  -M MAPFILE     Write file map of the generated disk image to MAPFILE. The\n"
"                 map file format is one line per file on disk:\n"
"                 [startTrack];[startSector];[filename]\n\n"
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
collectFiles(void)
{
    Diskfile *currentFile = 0;
    BlockPosition pos;
    const char *hostFileName;
    FILE *hostFile;

    do
    {
        switch(cmdline_opt(mkd64.cmdline))
        {
            case 'f':
                if (currentFile) diskfile_delete(currentFile);
                currentFile = 0;
                hostFileName = cmdline_arg(mkd64.cmdline);
                if (hostFileName)
                {
                    hostFile = fopen(hostFileName, "r");
                    if (hostFile)
                    {
                        currentFile = diskfile_new();
                        diskfile_readFromHost(currentFile, hostFile);
                        fclose(hostFile);
                    }
                    else
                    {
                        fprintf(stderr, "Error opening `%s' for reading: %s\n",
                                hostFileName, strerror(errno));
                    }
                }
                else
                {
                    currentFile = diskfile_new();
                }
                pos.track = 0;
                pos.sector = 0;
                break;
            case 't':
                if (currentFile) pos.track = atoi(cmdline_arg(mkd64.cmdline));
                break;
            case 's':
                if (currentFile) pos.sector = atoi(cmdline_arg(mkd64.cmdline));
                break;
            case 'i':
                if (currentFile) diskfile_setInterleave(currentFile,
                        atoi(cmdline_arg(mkd64.cmdline)));
                break;
            case 'w':
                if (currentFile)
                {
                    if (!diskfile_name(currentFile))
                        diskfile_setName(currentFile, hostFileName);
                    diskfile_write(currentFile, mkd64.image, &pos);
                    currentFile = 0;
                }
        }
        if (currentFile)
        {
            modrepo_allFileOption(mkd64.modrepo, currentFile,
                    cmdline_opt(mkd64.cmdline), cmdline_arg(mkd64.cmdline));
        }
    } while (cmdline_moveNext(mkd64.cmdline));
}

int
mkd64_run(void)
{
    int fileFound = 0;
    IModule *mod;

    if (!mkd64.initialized) return 0;

    if (!cmdline_moveNext(mkd64.cmdline))
    {
        printUsage();
        return 0;
    }

    if (cmdline_opt(mkd64.cmdline) == 'V')
    {
        printVersion();
        return 0;
    }

    if (cmdline_opt(mkd64.cmdline) == 'h')
    {
        printHelp(cmdline_arg(mkd64.cmdline));
        return 0;
    }

    do
    {
        switch (cmdline_opt(mkd64.cmdline))
        {
            case 'm':
                mod = modrepo_moduleInstance(mkd64.modrepo,
                        cmdline_arg(mkd64.cmdline));
                if (!mod)
                {
                    fprintf(stderr, "Error: module `%s' not found.\n",
                            cmdline_arg(mkd64.cmdline));
                    goto mkd64_run_error;
                }
                mod->initImage(mod, mkd64.image);
                break;
            case 'o':
                if (mkd64.d64)
                {
                    fputs("Error: D64 output file specified twice.\n", stderr);
                    goto mkd64_run_error;
                }
                mkd64.d64 = fopen(cmdline_arg(mkd64.cmdline), "w");
                if (!mkd64.d64)
                {
                    perror("Error opening D64 output file");
                    goto mkd64_run_error;
                }
                break;
            case 'M':
                if (mkd64.map)
                {
                    fputs("Error: file map output file specified twice.\n",
                            stderr);
                    goto mkd64_run_error;
                }
                mkd64.map = fopen(cmdline_arg(mkd64.cmdline), "w");
                if (!mkd64.map)
                {
                    perror("Error opening file map output file");
                    goto mkd64_run_error;
                }
                break;
            case 'f':
                fileFound = 1;
                break;
        }
        if (!fileFound)
        {
            modrepo_allGlobalOption(mkd64.modrepo,
                    cmdline_opt(mkd64.cmdline), cmdline_arg(mkd64.cmdline));
        }
    } while (!fileFound && cmdline_moveNext(mkd64.cmdline));

    if (fileFound) collectFiles();

    if (mkd64.d64)
    {
        if (!image_dump(mkd64.image, mkd64.d64))
            perror("Error writing D64 image");
        fclose(mkd64.d64);
        mkd64.d64 = 0;
    }

    if (mkd64.map)
    {
        if (!filemap_dump(image_filemap(mkd64.image), mkd64.map))
            perror("Error writing file map");
        fclose(mkd64.map);
        mkd64.map = 0;
    }

    return 1;

mkd64_run_error:
    if (mkd64.d64) fclose(mkd64.d64);
    if (mkd64.map) fclose(mkd64.map);
    return 0;
}

void
mkd64_done(void)
{
    if (!mkd64.initialized) return;
    mkd64.initialized = 0;
    modrepo_delete(mkd64.modrepo);
    cmdline_delete(mkd64.cmdline);
    image_delete(mkd64.image);
}

Image *
mkd64_image(void)
{
    return mkd64.initialized ? mkd64.image : 0;
}

Cmdline *
mkd64_cmdline(void)
{
    return mkd64.initialized ? mkd64.cmdline : 0;
}

Modrepo *
mkd64_modrepo(void)
{
    return mkd64.initialized ? mkd64.modrepo : 0;
}

int main(int argc, char **argv)
{
    mkd64_init(argc, argv);
    mkd64_run();
    mkd64_done();
}

/* vim: et:si:ts=4:sts=4:sw=4
*/
