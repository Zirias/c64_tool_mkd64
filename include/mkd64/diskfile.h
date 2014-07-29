#ifndef MKD64_DISKFILE_H
#define MKD64_DISKFILE_H

/** class DiskFile.
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <mkd64/common.h>

/** Class representing a file to be written on a C64 disk.
 * @class DiskFile mkd64/diskfile.h
 */
typedef struct DiskFile DiskFile;

#include <mkd64/image.h>
#include <mkd64/block.h>
#include <stdlib.h>

/** callback for deleting data attached using DiskFile_attachData().
 * @relates DiskFile
 * @param owner pointer to the owner as given to DiskFile_attachData()
 * @param data the data to delete
 */
typedef void (*DataDelete)(const void *owner, void *data);

/** Attach arbitrary data to this diskfile.
 * Each data owner should call this only once, otherwise the data from the
 * previous call will be deleted and a warning is printed to stderr.
 * @relates DiskFile
 * @param self the diskfile
 * @param owner the owner of the data to attach
 * @param data the data to attach
 * @param deleter a callback that must free all memory of data when called
 */
DECLEXPORT void DiskFile_attachData(DiskFile *self, const void *owner,
        void *data, DataDelete deleter);

/** Get attached data for a given owner.
 * @relates DiskFile
 * @param self the diskfile
 * @param owner the owner of the data
 * @return pointer to the data, or 0 if no data for this owner was found
 */
DECLEXPORT void *DiskFile_data(const DiskFile *self, const void *owner);

/** Get the size of the file in bytes.
 * @relates DiskFile
 * @param self the diskfile
 * @return size in bytes, or 0 if no real file was read from the local system
 */
DECLEXPORT size_t DiskFile_size(const DiskFile *self);

/** Get the size of the file in blocks.
 * this will only return a real value after the file was written to a disk
 * image, before that it will just return 0.
 * @relates DiskFile
 * @param self the diskfile
 * @return the size in blocks
 */
DECLEXPORT size_t DiskFile_blocks(const DiskFile *self);

/** Set interleave for this file.
 * this sets an interleave value to be used when the file is written to a disk
 * image. Modules may use this to set a default when they are told to handle
 * the file, the user may later override it by specifying a -i parameter before
 * the final -w. If this is never called, the file will be written to the disk
 * image without interleave (consecutive sectors).
 * @relates DiskFile
 * @param self the diskfile
 * @param interleave the interleave value to set.
 */
DECLEXPORT void DiskFile_setInterleave(DiskFile *self, int interleave);

/** Get interleave for this file.
 * @relates DiskFile
 * @param self the diskfile
 * @return the currently set interleave value
 */
DECLEXPORT int DiskFile_interleave(const DiskFile *self);

/** Set the name of this file.
 * mkd64 initially sets the file name to the name of the file loaded from the
 * local system -- or an empty string if no file was loaded. modules should
 * call this exactly when they create some kind of "directory entry" for the
 * file and set it to the name appearing in the directory. The file map feature
 * will use this name for creating the map (list of files with start
 * track/sector).
 * @relates DiskFile
 * @param self the diskfile
 * @param name the name to set for the file
 * @return 1, if the name was sucessfully parsed, 0 otherwise
 */
DECLEXPORT int DiskFile_setName(DiskFile *self, const char *name);

/** Get the name of this file.
 * @relates DiskFile
 * @param self the diskfile
 * @return the (current) name of the file, see DiskFile_setName().
 */
DECLEXPORT const char *DiskFile_name(const DiskFile *self);

/** Get the number of the file.
 * Each file gets automatically a number when it is written to the disk image,
 * so this can be used to determine the order in which files were written.
 * @relates DiskFile
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
