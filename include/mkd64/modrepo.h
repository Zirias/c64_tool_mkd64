#ifndef MKD64_MODREPO_H
#define MKD64_MODREPO_H

/** class ModRepo
 * @file
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <mkd64/common.h>

/** Module repository class.
 * handles searching for modules, loading and instantiating them and checking
 * dependencies and conflicts between modules, as well as calling functions
 * on all currently loaded modules
 * @class ModRepo mkd64/modrepo.h
 */
typedef struct ModRepo ModRepo;

#include <mkd64/imodule.h>

/** Get first instance of a module.
 * May be useful for getting the instance of a module that is known to have
 * only one instance.
 * @relates ModRepo
 * @param self the modrepo
 * @param id the id of the module
 * @return the module instance or 0 if not found and/or not loadable
 */
DECLEXPORT IModule *ModRepo_firstInstance(const ModRepo *self, const char *id);

/** Check wheter a given module is instantiated at least once.
 * @relates ModRepo
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
