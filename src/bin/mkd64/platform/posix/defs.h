#ifndef PLATFORM_DEFS_H
#define PLATFORM_DEFS_H

#ifdef __CYGWIN__
#define FINDPAT_MODULES "/*.dll"
#else
#define FINDPAT_MODULES "/*.so"
#endif
#undef APP_FILENAME
#define PATH_SEP "/"

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
