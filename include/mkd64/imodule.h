#ifndef MKD64_IMODULE_H
#define MKD64_IMODULE_H

#include <mkd64/common.h>

struct iModule;
/** runtime interface for mkd64 modules
 */
typedef struct iModule IModule;

#include <mkd64/image.h>
#include <mkd64/diskfile.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

struct iModule
{
    /** Get id of the module, for runtime identification
     * @return the module id
     */
    const char *(*id)(void);

    /** Delete the current module instance
     * @param this the module instance
     */
    void (*delete)(IModule *this);

    /** Initialize the module, called whenever the module is loaded
     * May be left unimplemented.
     * @param this the module instance
     * @param image the image to work on
     */
    void (*initImage)(IModule *this, Image *image);

    /** Handle a global cmdline option
     * May be left unimplemented.
     * @param this the module instance
     * @param opt the option
     * @param arg the option argument
     * @return 1 if the options is recognized by the module, 0 otherwise
     */
    int (*globalOption)(IModule *this, char opt, const char *arg);

    /** Handle a file cmdline option
     * May be left unimplemented.
     * @param this the module instance
     * @param file the file concerned by the option
     * @param opt the option
     * @param arg the option argument
     * @return 1 if the options is recognized by the module, 0 otherwise
     */
    int (*fileOption)(IModule *this,
            Diskfile *file, char opt, const char *arg);

    /** Get extra tracks, if provided by the module
     * May be left unimplemented.
     * @param this the module instance
     * @param track the number of the track to get
     * @return the track to the given number, or 0 if this module doesn't
     *  provide this track number
     */
    Track *(*getTrack)(IModule *this, int track);

    /** Called after a file was written to the disk image
     * May be left unimplemented.
     * Use this e.g. to write directory entries
     * @param this the module instance
     * @param file the file that was just written
     * @param start the starting track/sector position where the file was
     *  written
     */
    void (*fileWritten)(IModule *this,
            Diskfile *file, const BlockPosition *start);

    /** Called after the status of any block changed
     * May be left unimplemented.
     * Use this e.g. to write allocation maps
     * @param this the module instance
     * @param pos the position of the block that changed its status
     */
    void (*statusChanged)(IModule *this, const BlockPosition *pos);

    /** Called when someone else wants a block reserved by this module
     * May be left unimplemented. This has the same result as always returning
     * 0, all requests will be rejected.
     * @param this the module instance
     * @param pos the position of the reserved block requested
     * @return 1 if it is ok to give away this block, 0 otherwise.
     */
    int (*requestReservedBlock)(IModule *this, const BlockPosition *pos);

    /** Called after all files were written to the image
     * May be left unimplemented.
     * Use this e.g. to suggest better options or to print some stats to stdout.
     * @param this the module instance
     */
    void (*imageComplete)(IModule *this);
};

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
