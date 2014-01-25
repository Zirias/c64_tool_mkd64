#ifndef MKD64_IMODULE_H
#define MKD64_IMODULE_H

/** interface IModule.
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <mkd64/common.h>

/** Runtime interface for mkd64 modules.
 * @interface IModule mkd64/imodule.h
 */
typedef struct IModule IModule;

#include <mkd64/image.h>
#include <mkd64/diskfile.h>
#include <mkd64/track.h>
#include <mkd64/block.h>

struct IModule
{
    /** Get id of the module, for runtime identification.
     * @return the module id
     */
    const char *(*id)(void);

    /** Delete the current module instance
     * @param self the module instance
     */
    void (*free)(IModule *self);

    /** Initialize the module, called whenever the module is loaded.
     * May be left unimplemented.
     * @param self the module instance
     * @param image the image to work on
     */
    void (*initImage)(IModule *self, Image *image);

    /** Handle an option
     * Options specifically given for the module are passed here.
     * May be left unimplemented.
     * @param self the module instance
     * @param opt the option
     * @param arg the option argument
     * @return 1 if the option is recognized by the module, 0 otherwise
     */
    int (*option)(IModule *self, char opt, const char *arg);

    /** Handle a global cmdline option.
     * May be left unimplemented.
     * @param self the module instance
     * @param opt the option
     * @param arg the option argument
     * @return 1 if the option is recognized by the module, 0 otherwise
     */
    int (*globalOption)(IModule *self, char opt, const char *arg);

    /** Handle a file cmdline option.
     * May be left unimplemented.
     * @param self the module instance
     * @param file the file concerned by the option
     * @param opt the option
     * @param arg the option argument
     * @return 1 if the option is recognized by the module, 0 otherwise
     */
    int (*fileOption)(IModule *self,
            DiskFile *file, char opt, const char *arg);

    /** Get extra tracks, if provided by the module.
     * May be left unimplemented.
     * @param self the module instance
     * @param track the number of the track to get
     * @return the track to the given number, or 0 if this module doesn't
     *  provide this track number
     */
    Track *(*getTrack)(IModule *self, int track);

    /** Called after a file was written to the disk image.
     * May be left unimplemented.
     * Use this e.g. to write directory entries
     * @param self the module instance
     * @param file the file that was just written
     * @param start the starting track/sector position where the file was
     *  written
     */
    void (*fileWritten)(IModule *self,
            DiskFile *file, const BlockPosition *start);

    /** Called after the status of any block changed.
     * May be left unimplemented.
     * Use this e.g. to write allocation maps
     * @param self the module instance
     * @param pos the position of the block that changed its status
     */
    void (*statusChanged)(IModule *self, const BlockPosition *pos);

    /** Called when someone else wants a block reserved by this module.
     * May be left unimplemented. This has the same result as always returning
     * 0, all requests will be rejected.
     * @param self the module instance
     * @param pos the position of the reserved block requested
     * @return 1 if it is ok to give away this block, 0 otherwise.
     */
    int (*requestReservedBlock)(IModule *self, const BlockPosition *pos);

    /** Called after all files were written to the image.
     * May be left unimplemented.
     * Use this e.g. to suggest better options or to print some stats to stdout.
     * @param self the module instance
     */
    void (*imageComplete)(IModule *self);
};

#ifdef __cplusplus
}
#endif

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
