#ifndef MKD64_COMMON_H
#define MKD64_COMMON_H

#ifdef WIN32
#define SOEXPORT __declspec(dllexport)
#ifdef BUILDING_MKD64
#define DECLEXPORT __declspec(dllexport)
#else
#define DECLEXPORT __declspec(dllimport)
#endif
#define SOLOCAL
#else
#define DECLEXPORT
#if __GNUC__ >= 4
#define SOEXPORT __attribute__ ((visibility ("default")))
#define SOLOCAL __attribute__ ((visibility ("hidden")))
#else
#define SOEXPORT
#define SOLOCAL
#endif
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#include <stdint.h>
#else
typedef unsigned char uint8_t;
typedef unsigned int uintptr_t;
#endif

#if defined(_MSC_VER)
#define snprintf _snprintf
#endif

#define API_VER_MAJOR 1
#define API_VER_MINOR 3

#define API_VER_BETA

#ifndef BUILDING_MKD64
#define MKD64_MODULE(modname) static const int _mkd64_module_apiver[] = \
    { API_VER_MAJOR, API_VER_MINOR }; \
\
SOEXPORT const int *mkd64ApiVersion(void) \
{ \
    return _mkd64_module_apiver; \
} \
static const char *_modid = modname; \
SOEXPORT const char *id(void) \
{ \
    return _modid; \
}
#endif

#include <mkd64/util.h>

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

#define OBJDEL(classname, object) do { \
    classname##_done(object); \
    free(object); \
} while(0)

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
