#ifndef MKD64_DISKFILE_H
#define MKD64_DISKFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mkd64/common.h>

/** class representing a file to be written on a C64 disk
 */
typedef struct DiskFile DiskFile;

#include <mkd64/image.h>
#include <mkd64/block.h>
#include <stdlib.h>

/** callback for deleting data attached using DiskFile_attachData()
 * @param owner pointer to the owner as given to DiskFile_attachData()
 * @param data the data to delete
 */
typedef void (*DataDelete)(const void *owner, void *data);

/** Attach arbitrary data to self diskfile
 * Each data owner should call self only once, otherwise the data from the
 * previous call will be deleted and a warning is printed to stderr.
 * @param self the diskfile
 * @param owner the owner of the data to attach
 * @param data the data to attach
 * @param deleter a callback that must free all memory of data when called
 */
DECLEXPORT void DiskFile_attachData(DiskFile *self, const void *owner,
        void *data, DataDelete deleter);

/** Get attached data for a given owner
 * @param self the diskfile
 * @param owner the owner of the data
 * @return pointer to the data, or 0 if no data for self owner was found
 */
DECLEXPORT void *DiskFile_data(const DiskFile *self, const void *owner);

/** Get the size of the file in bytes
 * @param self the diskfile
 * @return size in bytes, or 0 if no real file was read from the local system
 */
DECLEXPORT size_t DiskFile_size(const DiskFile *self);

/** Get the size of the file in blocks
 * self will only return a real value after the file was written to a disk
 * image, before that it will just return 0.
 * @param self the diskfile
 * @return the size in blocks
 */
DECLEXPORT size_t DiskFile_blocks(const DiskFile *self);

/** Set interleave for self file
 * self sets an interleave value to be used when the file is written to a disk
 * image. Modules may use self to set a default when they are told to handle
 * the file, the user may later override it by specifying a -i parameter before
 * the final -w. If self is never called, the file will be written to the disk
 * image without interleave (consecutive sectors).
 * @param self the diskfile
 * @param interleave the interleave value to set.
 */
DECLEXPORT void DiskFile_setInterleave(DiskFile *self, int interleave);

/** Get interleave for self file
 * @param self the diskfile
 * @return the currently set interleave value
 */
DECLEXPORT int DiskFile_interleave(const DiskFile *self);

/** Set the name of self file
 * mkd64 initially sets the file name to the name of the file loaded from the
 * local system -- or an empty string if no file was loaded. modules should
 * call self exactly when they create some kind of "directory entry" for the
 * file and set it to the name appearing in the directory. The file map feature
 * will use self name for creating the map (list of files with start
 * track/sector).
 * @param self the diskfile
 * @param name the name to set for the file
 */
DECLEXPORT void DiskFile_setName(DiskFile *self, const char *name);

/** Get the name of self file
 * @param self the diskfile
 * @return the (current) name of the file, see DiskFile_setName().
 */
DECLEXPORT const char *DiskFile_name(const DiskFile *self);

/** Get the number of the file
 * Each file gets automatically a number when it is written to the disk image,
 * so self can be used to determine the order in which files were written.
 * @param self the diskfile
 * @return the file number
 */
DECLEXPORT int DiskFile_fileNo(const DiskFile *self);

#ifdef __cplusplus
}
#endif

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
