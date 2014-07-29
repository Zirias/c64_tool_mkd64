#ifndef UTIL_H
#define UTIL_H

#include <mkd64/util.h>

/** callback for getAppDir() when the application filename doesn't match
 * This is called by getAppDir() when it was given an expectedAppNameEnd
 * parameter and the actual application filename doesn't match it.
 * @param caller the pointer to the caller object, given to getAppDir()
 * @param appName the actual filename of the running application
 */
typedef void (*AppNameMismatchCallback)(void *caller, const char *appName);

/** get the directory of the running application
 * If an expectedAppNameEnd parameter is given, this only succeeds when the
 * application filename ends with [expectedAppNameEnd].
 * @param expectedAppNameEnd the expected end of the application filename
 * (including its full path), or 0 if not relevant
 * @param caller pointer to the calling object
 * @param mismatch callback to call when the application filename doesn't
 * match [expectedAppNameEnd]. May be 0.
 * @return the directory of the running application, dynamically allocated.
 * The caller is responsible to free the result.
 */
char *getAppDir(const char *expectedAppNameEnd,
        void *caller, AppNameMismatchCallback mismatch);

/** load a shared object
 * @param name the file name of the shared object to load
 * @return the platform-specific handle to the shared object or 0 on error
 */
void *loadDso(const char *name);

/** get function pointer from a shared object
 * @param dso the platform-specific handle to the shared object
 * @param name the name of the function in the shared object
 * @return a pointer to the function or 0 if not found
 */
void *getDsoFunction(void *dso, const char *name);

/** unload a shared object
 * @param dso the platform-specific handle to the shared object
 */
void unloadDso(void *dso);

#endif
