#ifndef MKD64_CMDLINE_H
#define MKD64_CMDLINE_H

struct cmdline;
typedef struct cmdline Cmdline;

DECLEXPORT char cmdline_opt(const Cmdline *this);
DECLEXPORT const char *cmdline_arg(const Cmdline *this);
DECLEXPORT const char *cmdline_exe(const Cmdline *this);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
