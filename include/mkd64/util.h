#ifndef MKD64_UTIL_H
#define MKD64_UTIL_H

/** Collection of generic utility functions for mkd64 and modules.
 * This file contains utility functions that do not directly belong to one
 * of mkd64's classes.
 * @file
 */

#include <mkd64/common.h>
#include <stdlib.h>
#include <stdio.h>

/** Callback for the findFilesInDir() function.
 * This is called by findFilesInDir() function for every file found.
 * @param caller the pointer to the caller object, given to findFilesInDir()
 * @param filename the name of the file found
 */
typedef void (*FileFoundCallback)(void *caller, const char *filename);

/** Get random integer in a given range.
 * The random number generator is initialized on the first call
 * @param min minimum number to return
 * @param max maximum number to return
 * @return random number
 */
DECLEXPORT int randomNum(int min, int max);

/** Try parsing an integer from a given string.
 * For tryParseInt, the string must contain a number represented only by
 * decimal digits and an optional minus ('-') at the beginning. Otherwise
 * it will return false and the content of the result is undefined.
 * @param str the string to parse
 * @param result pointer to an integer for placing the result
 * @return 1 (true) if parsed correctly, 0 (false) otherwise
 */
DECLEXPORT int tryParseInt(const char *str, int *result);

/** Try parsing an integer from a given hexadecimal string.
 * For tryParseIntHex, the string must contain a number represented only by
 * hexadecimal digits (0-9, a-z). Otherwise it will return false and the
 * content of the result is undefined.
 * @param str the string to parse
 * @param result pointer to an unsigned integer for placing the result
 * @return 1 (true) if parsed correctly, 0 (false) otherwise
 */
DECLEXPORT int tryParseIntHex(const char *str, unsigned int *result);

/** Check for presence of a argument and emit warning message.
 * This is a convenience function that checks whether an option has an
 * argument and directly prints a warning message, if this is not the
 * expected case. For missing arguments, the message will warn that the
 * option will be ignored, for extra arguments, the message will read that
 * the argument will be ignored. This is the recommended behavior for mkd64
 * modules.
 * @param opt the option
 * @param arg the actual argument (or, 0)
 * @param isFileOpt 1 if this is a file option, 0 if it is a global option
 * @param argExpected 1 if the option needs an argument, 0 if not
 * @param modid the id string of the module, or 0 if the caller is mkd64
 * @return 1 if expectation is met, 0 if not (and warning was printed)
 */
DECLEXPORT int checkArgAndWarn(char opt, const char *arg, int isFileOpt,
        int argExpected, const char *modid);

/** Take a copy of an existing string.
 * Use this instead of POSIX strdup() for C standard compliance
 * @arg s the string to copy
 * @return a copy, must be free()d by the caller
 */
DECLEXPORT char *copyString(const char *s);

/** Parse Escapes for hex numbers in a string.
 * This function works in-place.
 * @arg s the string to parse the escape values from
 * @return 1 (true) if parsed correctly, 0 (false) otherwise
 */
DECLEXPORT int parseHexEscapes(char *s);

/** Check whether a string ends with an expected content.
 * @param s the string to check
 * @param expectedEnd the content to check for at the end of the string
 * @param ignoreCase 0 to compare case-sensitive, 1 to ignore case
 * @return 1 if the string ends with expectedEnd, 0 otherwise
 */
DECLEXPORT int stringEndsWith(const char *s, const char *expectedEnd,
        int ignoreCase);

/** Allocate memory and fail instantly on error.
 * Memory allocation should only fail in out-of-memory conditions. In this
 * case, the safest thing to do for mkd64 is to fail instantly and exit with
 * an appropriate message.
 * @arg size the size of the memory block to allocate
 * @return the allocated memory
 */
DECLEXPORT void *mkd64Alloc(size_t size);

/** Find files matching a pattern in a given directory.
 * This will search a given directory for files matching a pattern
 * non-recursively and call a given callback for each file found.
 * ATTENTION: The exact pattern syntax depends on the platform-specific
 * implementation. An asterisk (*) should normally work as expected.
 * @param dir the directory to search files in
 * @param pattern the pattern the files should match
 * @param caller pointer to the calling object
 * @param found callback to call for files found
 */
DECLEXPORT void findFilesInDir(const char *dir, const char *pattern,
        void *caller, FileFoundCallback found);

/** get the size of a file
 * @param the file
 * @return the size of the file, -1 on error
 */
DECLEXPORT int64_t getFileSize(const FILE *file);

#endif
/* vim: et:si:ts=4:sts=4:sw=4
*/
