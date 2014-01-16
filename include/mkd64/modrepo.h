#ifndef MKD64_MODREPO_H
#define MKD64_MODREPO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mkd64/common.h>

/** Module repository class
 * handles searching for modules, loading and instantiating them and checking
 * dependencies and conflicts between modules, as well as calling functions
 * on all currently loaded modules
 */
typedef struct ModRepo ModRepo;

#include <mkd64/imodule.h>

/** Get instance of a module
 * if the module is not currently instantiated, Modrepo tries to instantiate
 * it first.
 * @param self the modrepo
 * @param id the id of the module
 * @return the module instance or 0 if not found and/or not loadable
 */
DECLEXPORT IModule *ModRepo_moduleInstance(ModRepo *self, const char *id);

/** Check wheter a given module is instantiated
 * @param self the modrepo
 * @param id the id of the module
 * @return 1 if module is active (instantiated), 0 otherwise
 */
DECLEXPORT int ModRepo_isActive(const ModRepo *self, const char *id);

#ifdef __cplusplus
}
#endif

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
