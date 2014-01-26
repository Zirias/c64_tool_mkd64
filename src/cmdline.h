#ifndef CMDLINE_H
#define CMDLINE_H

#include <stdio.h>

typedef struct Cmdline Cmdline;

size_t Cmdline_objectSize(void);
Cmdline *Cmdline_init(Cmdline *self);
void Cmdline_done(Cmdline *self);

void Cmdline_parse(Cmdline *self, int argc, char **argv);
int Cmdline_parseFile(Cmdline *self, const char *cmdfile);
char Cmdline_opt(const Cmdline *self);
const char *Cmdline_arg(const Cmdline *self);
int Cmdline_moveNext(Cmdline *self);
const char *Cmdline_exe(const Cmdline *self);
int Cmdline_count(const Cmdline *self);

#endif
/* vim: et:si:ts=8:sts=4:sw=4
*/
