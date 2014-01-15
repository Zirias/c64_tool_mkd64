#ifndef CMDLINE_H
#define CMDLINE_H

#include <stdio.h>

typedef struct Cmdline Cmdline;

size_t Cmdline_objectSize(void);
Cmdline *Cmdline_init(Cmdline *this);
void Cmdline_done(Cmdline *this);

void Cmdline_parse(Cmdline *this, int argc, char **argv);
int Cmdline_parseFile(Cmdline *this, const char *cmdfile);
char Cmdline_opt(const Cmdline *this);
const char *Cmdline_arg(const Cmdline *this);
int Cmdline_moveNext(Cmdline *this);
const char *Cmdline_exe(const Cmdline *this);
int Cmdline_count(const Cmdline *this);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
