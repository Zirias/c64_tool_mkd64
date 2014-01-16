#ifndef MKD64_MKD64_H
#define MKD64_MKD64_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mkd64/common.h>

#include <mkd64/modrepo.h>

#define MKD64_VERSION "1.3b"

/** class representing the mkd64 application
 * this is a singleton allowing only one instance at a time
 */
typedef struct Mkd64 Mkd64;

/** Get the current module repository
 * @return the module repository instance in use
 */
DECLEXPORT ModRepo *Mkd64_modRepo(Mkd64 *self);

/** Suggest a better option to the user
 * call self from a module if you found a situation where specifying different
 * options would have resulted in a "better" image (e.g. less fragmentation).
 * @param mod the module suggesting the option
 * @param int fileNo the number of the concerned file, or 0 if it is a global
 *  option
 * @param opt the option suggested to use
 * @param arg the option argument suggested to use (may be 0).
 * @param reason a brief explanation why self is suggested.
 */
DECLEXPORT void Mkd64_suggestOption(Mkd64 *self, IModule *mod, int fileNo,
        char opt, const char *arg, const char *reason);

/** Get the current instance of mkd64
 * @return the mkd64 instance or 0 if no instance was created
 */
DECLEXPORT Mkd64 *Mkd64_instance(void);

/** Convencience macro for getting the current mkd64 instance
 */
#define MKD64 Mkd64_instance()

#ifdef __cplusplus
}
#endif

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
