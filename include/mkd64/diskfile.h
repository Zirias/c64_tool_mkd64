#ifndef MKD64_DISKFILE_H
#define MKD64_DISKFILE_H

#include <mkd64/common.h>

struct diskfile;
/** class representing a file to be written on a C64 disk
 */
typedef struct diskfile Diskfile;

#include <mkd64/image.h>
#include <mkd64/block.h>
#include <stdlib.h>

/** callback for deleting data attached using diskfile_attachData()
 * @param owner pointer to the owner as given to diskfile_attachData()
 * @param data the data to delete
 */
typedef void (*DataDelete)(const void *owner, void *data);

/** Attach arbitrary data to this diskfile
 * Each data owner should call this only once, otherwise the data from the
 * previous call will be deleted and a warning is printed to stderr.
 * @param this the diskfile
 * @param owner the owner of the data to attach
 * @param data the data to attach
 * @param deleter a callback that must free all memory of data when called
 */
DECLEXPORT void diskfile_attachData(Diskfile *this, const void *owner,
        void *data, DataDelete deleter);

/** Get attached data for a given owner
 * @param this the diskfile
 * @param owner the owner of the data
 * @return pointer to the data, or 0 if no data for this owner was found
 */
DECLEXPORT void *diskfile_data(const Diskfile *this, const void *owner);

/** Get the size of the file in bytes
 * @param this the diskfile
 * @return size in bytes, or 0 if no real file was read from the local system
 */
DECLEXPORT size_t diskfile_size(const Diskfile *this);

/** Get the size of the file in blocks
 * This will only return a real value after the file was written to a disk
 * image, before that it will just return 0.
 * @param this the diskfile
 * @return the size in blocks
 */
DECLEXPORT size_t diskfile_blocks(const Diskfile *this);

/** Set interleave for this file
 * This sets an interleave value to be used when the file is written to a disk
 * image. Modules may use this to set a default when they are told to handle
 * the file, the user may later override it by specifying a -i parameter before
 * the final -w. If this is never called, the file will be written to the disk
 * image without interleave (consecutive sectors).
 * @param this the diskfile
 * @param interleave the interleave value to set.
 */
DECLEXPORT void diskfile_setInterleave(Diskfile *this, int interleave);

/** Get interleave for this file
 * @param this the diskfile
 * @return the currently set interleave value
 */
DECLEXPORT int diskfile_interleave(const Diskfile *this);

/** Set the name of this file
 * mkd64 initially sets the file name to the name of the file loaded from the
 * local system -- or an empty string if no file was loaded. modules should
 * call this exactly when they create some kind of "directory entry" for the
 * file and set it to the name appearing in the directory. The file map feature
 * will use this name for creating the map (list of files with start
 * track/sector).
 * @param this the diskfile
 * @param name the name to set for the file
 */
DECLEXPORT void diskfile_setName(Diskfile *this, const char *name);

/** Get the name of this file
 * @param this the diskfile
 * @return the (current) name of the file, see diskfile_setName().
 */
DECLEXPORT const char *diskfile_name(const Diskfile *this);

/** Get the number of the file
 * Each file gets automatically a number when it is written to the disk image,
 * so this can be used to determine the order in which files were written.
 * @param this the diskfile
 * @return the file number
 */
DECLEXPORT int diskfile_fileNo(const Diskfile *this);

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
