#ifndef MKD64_MODULE_H
#define MKD64_MODULE_H

#include "imodule.h"

#ifdef WIN32
#define MOD_STATIC __declspec(dllexport)
#else
#define MOD_STATIC
#endif

MOD_STATIC const char *id(void);
MOD_STATIC IModule *instance(void);
MOD_STATIC void delete(IModule *instance);

#endif

/* vim: et:si:ts=8:sts=4:sw=4
*/
