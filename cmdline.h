#ifndef MKD64_CMDLINE_H
#define MKD64_CMDLINE_H

struct cmdline;
typedef struct cmdline Cmdline;

Cmdline *cmdline_new(void);
void cmdline_delete(Cmdline *this);

void cmdline_parse(Cmdline *this, int argc, char **argv);
char cmdline_opt(const Cmdline *this);
const char *cmdline_arg(const Cmdline *this);
int cmdline_moveNext(Cmdline *this);

const char *cmdline_exe(const Cmdline *this);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
