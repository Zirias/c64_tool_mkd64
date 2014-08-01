#ifndef MKD64_COMMON_H
#define MKD64_COMMON_H

/** Common mkd64 defines and macros.
 * Defines for exporting/importing symbols, helper macros for modules and
 * object handling and some miscellaneous stuff. Should always be included.
 * @file
 */

/** Example for a simple module skeleton.
 * This demonstrates the minimal use of the API to create a module that can
 * be loaded by mkd64. It does nothing.
 * @example module.c
 */

#ifdef __cplusplus
#define mkd64___cdecl extern "C"
#else
#define mkd64___cdecl
#endif

#ifdef WIN32
#define SOEXPORT mkd64___cdecl __declspec(dllexport)
#ifdef BUILDING_MKD64
#define DECLEXPORT __declspec(dllexport)
#else
#define DECLEXPORT mkd64___cdecl __declspec(dllimport)
#endif
#define SOLOCAL
#else
#define DECLEXPORT mkd64___cdecl
#if __GNUC__ >= 4
#define SOEXPORT mkd64___cdecl __attribute__ ((visibility ("default")))
#define SOLOCAL __attribute__ ((visibility ("hidden")))
#else
#define SOEXPORT mkd64___cdecl
#define SOLOCAL
#endif
#endif

#if defined (__cplusplus) || (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#include <stdint.h>
#else
/* rough estimations, provide at least the required size */
typedef unsigned char uint8_t;
typedef unsigned int uintptr_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif

#ifndef SIZE_MAX
#define SIZE_MAX (~(size_t)0)
#endif

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

#define API_VER_MAJOR 1
#define API_VER_MINOR 4

#define API_VER_BETA

#ifndef BUILDING_MKD64
/** Declare a module.
 * This must be called once in every mkd64 module. Do NOT place a semicolon at
 * the end.
 * @see module.c
 * @param modname the name (id) of the module
 */
#define MKD64_MODULE(modname) \
static const int mkd64___module_apiver[] = \
    { API_VER_MAJOR, API_VER_MINOR }; \
\
SOEXPORT const int *mkd64ApiVersion(void) \
{ \
    return mkd64___module_apiver; \
} \
static const char *mkd64___modid = modname; \
SOEXPORT const char *id(void) \
{ \
    return mkd64___modid; \
}

#endif

#include <mkd64/util.h>

/** Create a new object instance
 * There are variations OBJNEW1 to OBJNEW5 taking 1 to 5 additional parameters
 * that are passed to the constructor.
 * @param classname the name of the class for the new object
 */
#define OBJNEW(classname) classname##_init( \
        (classname *)mkd64Alloc(classname##_objectSize()))

#define OBJNEW1(classname, p1) classname##_init( \
        (classname *)mkd64Alloc(classname##_objectSize()), p1)

#define OBJNEW2(classname, p1, p2) classname##_init( \
        (classname *)mkd64Alloc(classname##_objectSize()), p1, p2)

#define OBJNEW3(classname, p1, p2, p3) classname##_init( \
        (classname *)mkd64Alloc(classname##_objectSize()), p1, p2, p3)

#define OBJNEW4(classname, p1, p2, p3, p4) classname##_init( \
        (classname *)mkd64Alloc(classname##_objectSize()), p1, p2, p3, p4)

#define OBJNEW5(classname, p1, p2, p3, p4, p5) classname##_init( \
        (classname *)mkd64Alloc(classname##_objectSize()), p1, p2, p3, p4, p5)

/** Delete an object instance
 * This calls the destructor for the object and frees allocated memory.
 * @param classname the class name of the object
 * @param object the pointer to the object
 */
#define OBJDEL(classname, object) do { \
    classname##_done(object); \
    free(object); \
} while(0)

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
