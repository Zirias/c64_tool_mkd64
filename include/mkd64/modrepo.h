#ifndef MKD64_MODREPO_H
#define MKD64_MODREPO_H

#include <mkd64/common.h>

struct modrepo;
/** Module repository class
 * handles searching for modules, loading and instantiating them and checking
 * dependencies and conflicts between modules, as well as calling functions
 * on all currently loaded modules
 */
typedef struct modrepo Modrepo;

#include <mkd64/imodule.h>

/** Get instance of a module
 * if the module is not currently instantiated, Modrepo tries to instantiate
 * it first.
 * @param this the modrepo
 * @param id the id of the module
 * @return the module instance or 0 if not found and/or not loadable
 */
DECLEXPORT IModule *modrepo_moduleInstance(Modrepo *this, const char *id);

/** Check wheter a given module is instantiated
 * @param this the modrepo
 * @param id the id of the module
 * @return 1 if module is active (instantiated), 0 otherwise
 */
DECLEXPORT int modrepo_isActive(const Modrepo *this, const char *id);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
