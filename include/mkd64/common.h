#ifndef MKD64_COMMON_H
#define MKD64_COMMON_H

#ifdef WIN32
#define SOEXPORT __declspec(dllexport)
#define SOLOCAL
#else
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
#endif

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
