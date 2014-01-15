#ifndef MKD64_MKD64_H
#define MKD64_MKD64_H

#include <mkd64/common.h>

#include <mkd64/modrepo.h>

#define MKD64_VERSION "1.3b"

typedef struct Mkd64 Mkd64;

/** Get the current module repository
 * @return the module repository instance in use
 */
DECLEXPORT ModRepo *Mkd64_modRepo(Mkd64 *this);

/** Suggest a better option to the user
 * call this from a module if you found a situation where specifying different
 * options would have resulted in a "better" image (e.g. less fragmentation).
 * @param mod the module suggesting the option
 * @param int fileNo the number of the concerned file, or 0 if it is a global
 *  option
 * @param opt the option suggested to use
 * @param arg the option argument suggested to use (may be 0).
 * @param reason a brief explanation why this is suggested.
 */
DECLEXPORT void Mkd64_suggestOption(Mkd64 *this, IModule *mod, int fileNo,
        char opt, const char *arg, const char *reason);

DECLEXPORT Mkd64 *Mkd64_instance(void);

#define MKD64 Mkd64_instance()

#endif

/* vim: et:si:ts=4:sts=4:sw=4
*/
