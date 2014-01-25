#ifndef MKD64_MKD64_H
#define MKD64_MKD64_H

/** class Mkd64
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <mkd64/common.h>

#include <mkd64/modrepo.h>

#define MKD64_VERSION "1.3b"

/** Class representing the mkd64 application.
 * This is a singleton allowing only one instance at a time.
 * @relates Mkd64
 * @class Mkd64 mkd64/mkd64.h
 */
typedef struct Mkd64 Mkd64;

/** Get the current module repository.
 * @relates Mkd64
 * @param self the Mkd64 instance
 * @return the module repository instance in use
 */
DECLEXPORT ModRepo *Mkd64_modRepo(Mkd64 *self);

/** Suggest a better option to the user.
 * Call this from a module if you found a situation where specifying different
 * options would have resulted in a "better" image (e.g. less fragmentation).
 * @relates Mkd64
 * @param self the Mkd64 instance
 * @param mod the module suggesting the option
 * @param fileNo the number of the concerned file, or 0 if it is a module
 *  option
 * @param opt the option suggested to use
 * @param arg the option argument suggested to use (may be 0).
 * @param reason a brief explanation why this is suggested.
 */
DECLEXPORT void Mkd64_suggestOption(Mkd64 *self, IModule *mod, int fileNo,
        char opt, const char *arg, const char *reason);

/** Get the current instance of mkd64.
 * @relates Mkd64
 * @return the mkd64 instance or 0 if no instance was created
 */
DECLEXPORT Mkd64 *Mkd64_instance(void);

/** Convencience macro for getting the current mkd64 instance.
 * @relates Mkd64
 */
#define MKD64 Mkd64_instance()

#ifdef __cplusplus
}
#endif

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
